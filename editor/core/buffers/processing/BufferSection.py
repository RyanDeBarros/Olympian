from . import Logger, DeferredExclam


class BufferParseStructure:
	META_BLOCK_DELIMITER = "---"
	COMMENT_PREFIX = '#'
	TITLE_PREFIX = ';'
	FIELD_SEPARATOR = ':'
	FIELD_METADATA_PREFIX = '-'
	EXCLAM_PREFIX = '!'

	class Line:
		def __init__(self, line: str, idx: int, row: int, cumulative_flat_idx: int):
			self.idx = idx
			self.row = row
			self.cumulative_flat_idx = cumulative_flat_idx

			result = ""
			in_quote = False
			escaping = False
			for c in line:
				if in_quote:
					if escaping:
						escaping = False
					elif c == '\\':
						escaping = True
					elif c == '"':
						in_quote = False
				elif c == '"':
					in_quote = True
				elif c == BufferParseStructure.COMMENT_PREFIX:
					break
				result += c

			self.line = result.lstrip()
			self.col_offset = len(result) - len(self.line)
			self.line = self.line.rstrip()

		def col(self, col: int) -> int:
			return col + self.col_offset

		def is_section_title(self) -> bool:
			return self.line.startswith(BufferParseStructure.TITLE_PREFIX)

		def section_indent_level(self) -> int:
			level = 0
			for c in self.line:
				if c == BufferParseStructure.TITLE_PREFIX:
					level += 1
				else:
					break
			return level

		def is_field_assignment(self) -> bool:
			return BufferParseStructure.FIELD_SEPARATOR in self.line and not self.line.startswith(BufferParseStructure.FIELD_METADATA_PREFIX)

		def split_field_assignment(self) -> tuple[str, str]:
			field, value = self.line.split(BufferParseStructure.FIELD_SEPARATOR, maxsplit=1)
			return field.strip(), value.strip()

	def __init__(self, lines: list[str]):
		self.old_lines = lines
		self.lines: list[BufferParseStructure.Line] = []
		row = 0
		idx = 0
		in_meta_block = False
		cumulative_flat_idx = 0
		for line in self.old_lines:
			line_length = len(line)
			line.strip()
			if line == self.META_BLOCK_DELIMITER:
				in_meta_block = not in_meta_block
			elif not in_meta_block:
				line = BufferParseStructure.Line(line, idx, row, cumulative_flat_idx)
				if len(line.line) > 0:
					self.lines.append(line)
					idx += 1

			cumulative_flat_idx += line_length + 1  # \n
			row += 1

		self.line_idx = 0

	def can_parse(self) -> bool:
		return self.line_idx < len(self.lines)

	def next_line(self) -> None:
		self.line_idx += 1

	def current_line(self) -> "BufferParseStructure.Line":
		return self.lines[self.line_idx]

	def skip_to_next_section(self) -> None:
		while self.can_parse() and not self.current_line().is_section_title():
			self.next_line()


class ValidationError(RuntimeError):
	def __init__(self, info: str, row: int, col: int):
		msg = f"({row},{col}): {info}"
		super().__init__(msg)
		self.msg = msg

	def log(self):
		Logger.log_error(self.msg)


class BufferSection:
	def __init__(self, name: str, parent: "BufferSection | None"):
		self.name = name
		self.parent = parent
		self.children: dict[str, BufferSection] = {}
		if self.parent:
			self.level = self.parent.level + 1
			self.parent.children[self.name] = self
		else:
			self.level = 0
		self.fields: dict[str, str] = {}

	def title(self) -> str:
		return f"{BufferParseStructure.TITLE_PREFIX * self.level} {self.name}"

	def repr_field(self, field: str) -> str:
		return f"{field}{BufferParseStructure.FIELD_SEPARATOR} {self.fields[field]}"

	@staticmethod
	def repr_metadata(metadata: str) -> str:
		return f"{BufferParseStructure.FIELD_METADATA_PREFIX} {metadata}"

	def add_deferred_commands(self, parse_structure: BufferParseStructure, deferred_exclams: list[DeferredExclam], line: BufferParseStructure.Line) -> None:
		class State:
			def __init__(self):
				self.exclam = ""
				self.in_exclam = False
				self.has_args = False
				self.in_quote = False
				self.escaping = False
				self.col = -1

		state = State()

		def add_deferred_command():
			state.in_exclam = False
			ctx = BufferSectionContext(parse_structure, line.idx, state.col - len(state.exclam), self)
			deferred_exclams.append(DeferredExclam(ctx, state.exclam))
			state.exclam = ""

		for c in line.line:
			state.col += 1
			if state.in_quote:
				if state.escaping:
					state.escaping = False
				elif c == '\\':
					state.escaping = True
				elif c == '"':
					state.in_quote = False
			elif c == '"':
				state.in_quote = True
			elif state.in_exclam > 0:
				if state.has_args:
					state.exclam += c
					if c == ')':
						add_deferred_command()
				elif c == ' ':
					add_deferred_command()
				elif c == '(':
					state.exclam += c
					state.has_args = True
				else:
					state.exclam += c
			elif c == BufferParseStructure.EXCLAM_PREFIX:
				state.in_exclam = True

		if state.in_exclam:
			add_deferred_command()


	def load_section(self, sections_list: list["BufferSection"], parse_structure: BufferParseStructure, deferred_exclams: list[DeferredExclam]) -> None:
		while parse_structure.can_parse():
			try:
				line = parse_structure.current_line()
				parse_structure.next_line()
				if line.is_section_title():
					level = line.section_indent_level()

					if level == self.level:  # sibling
						parent = self.parent
					elif level > self.level:  # child
						if level > self.level + 1:
							raise ValidationError("Missing intermediate child section", line.row, line.col(0))
						parent = self
					else:  # parent/grandparent
						degree = self.level - level
						parent = self.parent
						for _ in range(degree):
							parent = parent.parent

					section = BufferSection(line.line[level:].strip(), parent)
					sections_list.append(section)
					section.load_section(sections_list, parse_structure, deferred_exclams)

					continue

				if line.is_field_assignment():
					field, value = line.split_field_assignment()
					self.fields[field] = value
				else:
					# TODO v7.1 check for multi-line arguments
					self.add_deferred_commands(parse_structure, deferred_exclams, line)

			except ValidationError as e:
				e.log()
				parse_structure.skip_to_next_section()


class BufferSectionContext:
	def __init__(self, parse_structure: BufferParseStructure, line_idx: int, col: int, section: BufferSection):
		self.parse_structure = parse_structure
		self.line_idx = line_idx
		self.col = col
		self.section = section

	def line(self) -> BufferParseStructure.Line:
		return self.parse_structure.lines[self.line_idx]

	def flat_index(self) -> int:
		print(self.line().cumulative_flat_idx)
		return self.line().cumulative_flat_idx + self.line().col(self.col)
