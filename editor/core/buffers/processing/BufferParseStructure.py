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
