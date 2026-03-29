from editor.core import IntReference


class BufferSection:
	TITLE_PREFIX = ';'
	FIELD_SEPARATOR = ':'
	FIELD_METADATA_PREFIX = '-'

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
		return f"\n{BufferSection.TITLE_PREFIX * self.level} {self.name}"

	def repr_field(self, field: str) -> str:
		return f"{field}{self.FIELD_SEPARATOR} {self.fields[field]}"

	def repr_metadata(self, metadata: str) -> str:
		return f"{self.FIELD_METADATA_PREFIX} {metadata}"

	@staticmethod
	def strip_comments(line: str) -> str:
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
		return result.strip()

	def _process_new_section(self, sections_list: list["BufferSection"], line: str, lines: list[str], line_idx: IntReference) -> None:
		level = 0
		for c in line:
			if c == BufferSection.TITLE_PREFIX:
				level += 1
			else:
				break

		name = line[level:].strip()

		if level == self.level:  # sibling
			parent = self.parent
		elif level > self.level:  # child
			if level > self.level + 1:
				pass  # TODO v7.1 validation error - missing intermediate child section
			parent = self
		else:  # parent/grandparent
			degree = self.level - level
			parent = self.parent
			for _ in range(degree):
				parent = parent.parent

		section = BufferSection(name, parent)
		sections_list.append(section)
		section.load_section(sections_list, lines, line_idx)

	def _process_line(self, sections_list: list["BufferSection"], lines: list[str], line_idx: IntReference) -> bool:
		line = lines[line_idx.value]
		line_idx.value += 1
		if line.startswith(BufferSection.TITLE_PREFIX):
			self._process_new_section(sections_list, line, lines, line_idx)
			return False

		if ':' in line and not line.startswith(self.FIELD_METADATA_PREFIX):
			field, value = line.split(self.FIELD_SEPARATOR, maxsplit=1)
			field = field.strip()
			value = value.strip()
			self.fields[field] = value
		else:
			pass  # TODO v7.1 check for multi-line arguments OR !commands

		return True

	def load_section(self, sections_list: list["BufferSection"], lines: list[str], line_idx: IntReference):
		while line_idx.value < len(lines):
			if not self._process_line(sections_list, lines, line_idx):
				break


class BufferSectionContext:
	def __init__(self, section: BufferSection, *, field: str | None = None):
		self.section = section
		self.field = field
