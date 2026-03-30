import re
from typing import TYPE_CHECKING

from . import ExclamArgs, ExclamEdit

if TYPE_CHECKING:
	from . import ExclamContext, ExclamCommand


class DeferredExclam:
	def __init__(self, ctx: "ExclamContext", exclam: str):
		m = re.match(r"(?P<cmd>[^()]+)(?P<args>\(.*\))?", exclam)
		if not m:
			raise ValueError("Exclam pattern not supported")

		self.ctx = ctx
		self.cmd = m.group("cmd").strip()
		argline = m.group("args")
		self.args = ExclamArgs(argline.strip()) if argline is not None else ExclamArgs("")
		self.cmd_length = len(m.group(0))

	def valid(self, commands: list["ExclamCommand"]) -> bool:
		return any(command.cmd == self.cmd for command in commands)

	def invoke(self, commands: list["ExclamCommand"]) -> list[ExclamEdit]:
		for command in commands:
			if command.cmd == self.cmd:
				return command.fn(self.ctx, self.args)
		else:
			return []

	def removal(self) -> ExclamEdit:
		start_idx = self.ctx.flat_index()
		return ExclamEdit(start_idx, start_idx + self.cmd_length, "")
