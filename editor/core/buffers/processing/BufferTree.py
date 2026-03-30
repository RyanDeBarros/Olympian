from typing import TYPE_CHECKING

from . import BufferSection, BufferParseStructure, DeferredExclam

if TYPE_CHECKING:
	from . import ExclamCommand


class BufferTree:
	def __init__(self):
		self.root_section = BufferSection("", None)
		self.subsections: list[BufferSection] = []
		self.current_section = None

	def reset(self):
		self.root_section = BufferSection("", None)
		self.subsections.clear()
		self.current_section = self.root_section

	def rebuild(self, commands: list["ExclamCommand"], lines: list[str]) -> list[DeferredExclam]:
		self.reset()
		parse_structure = BufferParseStructure(lines)

		deferred_exclams: list[DeferredExclam] = []
		self.root_section.load_section(self.subsections, parse_structure, deferred_exclams)
		self.current_section = self.root_section

		kept_deferred_exclams: list[DeferredExclam] = []
		for deferred_exclam in deferred_exclams:
			if deferred_exclam.valid(commands):
				kept_deferred_exclams.append(deferred_exclam)

		return kept_deferred_exclams
