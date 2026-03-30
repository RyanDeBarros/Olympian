from typing import TYPE_CHECKING

from . import BufferParseStructure

if TYPE_CHECKING:
	from . import BufferSection


class ExclamContext:
	def __init__(self, parse_structure: BufferParseStructure, line_idx: int, col: int, section: "BufferSection"):
		self.parse_structure = parse_structure
		self.line_idx = line_idx
		self.col = col
		self.section = section

	def line(self) -> BufferParseStructure.Line:
		return self.parse_structure.lines[self.line_idx]

	def flat_index(self) -> int:
		return self.line().cumulative_flat_idx + self.line().col(self.col)
