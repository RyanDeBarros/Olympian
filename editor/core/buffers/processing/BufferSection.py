from . import Logger


class BufferParseStructure:
	META_BLOCK_DELIMITER = "---"
	COMMENT_PREFIX = '#'
	TITLE_PREFIX = ';'
	FIELD_SEPARATOR = ':'
	FIELD_METADATA_PREFIX = '-'

	class Line:
		def __init__(self, line: str, row: int):
			self.row = row

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
				elif c == '#':
					break
				result += c

			self.line = result.lstrip()
			self.col_offset = len(result) - len(self.line)
			self.line = self.line.rstrip()

		def char_col(self, col: int) -> int:
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
		row = -1
		in_meta_block = False
		for line in self.old_lines:
			row += 1
			line.strip()
			if line == self.META_BLOCK_DELIMITER:
				in_meta_block = not in_meta_block
			elif not in_meta_block:
				line = BufferParseStructure.Line(line, row)
				if len(line.line) > 0:
					self.lines.append(line)

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
		return f"\n{BufferParseStructure.TITLE_PREFIX * self.level} {self.name}"

	def repr_field(self, field: str) -> str:
		return f"{field}{BufferParseStructure.FIELD_SEPARATOR} {self.fields[field]}"

	@staticmethod
	def repr_metadata(metadata: str) -> str:
		return f"{BufferParseStructure.FIELD_METADATA_PREFIX} {metadata}"

	def load_section(self, sections_list: list["BufferSection"], parse_structure: BufferParseStructure) -> None:
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
							raise ValidationError("Missing intermediate child section", line.row, line.char_col(0))
						parent = self
					else:  # parent/grandparent
						degree = self.level - level
						parent = self.parent
						for _ in range(degree):
							parent = parent.parent

					section = BufferSection(line.line[level:].strip(), parent)
					sections_list.append(section)
					section.load_section(sections_list, parse_structure)

					continue

				if line.is_field_assignment():
					field, value = line.split_field_assignment()
					self.fields[field] = value
				else:
					pass  # TODO v7.1 check for multi-line arguments OR !commands

			except ValidationError as e:
				e.log()
				parse_structure.skip_to_next_section()


class BufferSectionContext:
	def __init__(self, parse_structure: BufferParseStructure, row: int, col: int, section: BufferSection, *, field: str | None = None):
		self.parse_structure = parse_structure
		self.row = row
		self.col = col
		self.section = section
		self.field = field
