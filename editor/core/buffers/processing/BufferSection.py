from . import DeferredExclam, BufferParseStructure, ExclamContext
from .. import Logger


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
			ctx = ExclamContext(parse_structure, line.idx, state.col - len(state.exclam), self)
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
					# TODO v7.1 check for multi-line arguments (like multi-line strings or arrays) -> just end previous line with \?
					self.add_deferred_commands(parse_structure, deferred_exclams, line)

			except ValidationError as e:
				e.log()
				parse_structure.skip_to_next_section()

