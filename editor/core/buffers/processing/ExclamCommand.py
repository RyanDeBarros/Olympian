import re
from io import StringIO
from typing import Callable, TYPE_CHECKING

if TYPE_CHECKING:
	from .BufferSection import BufferSectionContext


class ExclamArgs:
	def __init__(self, argline: str):
		self.kv_args: dict[str, str] = {}
		self.id_args: set[str] = set()
		for arg in argline.split(','):
			arg = arg.strip()
			if '=' in arg:
				k, v = arg.split('=', maxsplit=1)
				self.kv_args[k] = v
			else:
				self.id_args.add(arg)


class ExclamEdit:
	def __init__(self, start_idx: int, end_idx: int, new_text: str):
		assert start_idx <= end_idx
		self.start_idx = start_idx
		self.end_idx = end_idx
		self.new_text = new_text

	def _transform_index(self, idx: int) -> int:
		if idx <= self.start_idx:
			return idx

		delta = len(self.new_text) - (self.end_idx - self.start_idx)
		if idx >= self.end_idx:
			return idx + delta

		return min(idx, self.end_idx + delta)

	def invoke(self, fio: StringIO, queue: list["ExclamEdit"]) -> None:
		text = fio.getvalue()
		fio.seek(0)
		fio.truncate()
		fio.write(text[:self.start_idx] + self.new_text + text[self.end_idx + 1:])
		for edit in queue:
			edit.start_idx = self._transform_index(edit.start_idx)
			edit.end_idx = self._transform_index(edit.end_idx)


class ExclamCommand:
	def __init__(self, cmd: str, fn: Callable[["BufferSectionContext", ExclamArgs], list[ExclamEdit]], info: str, args: str | None = None):
		self.cmd = cmd
		self.fn = fn
		self.info = info
		self.exclam = f"!{cmd}" if args is None else f"!{cmd}({args})"


class DeferredExclam:
	def __init__(self, ctx: "BufferSectionContext", exclam: str):
		m = re.match(r"(?P<cmd>[^()]+)(?P<args>\(.*\))?", exclam)
		if not m:
			raise ValueError("Exclam pattern not supported")

		self.ctx = ctx
		self.cmd = m.group("cmd").strip()
		argline = m.group("args")
		self.args = ExclamArgs(argline.strip()) if argline is not None else ExclamArgs("")
		self.cmd_length = len(m.group(0))

	def valid(self, commands: list[ExclamCommand]) -> bool:
		return any(command.cmd == self.cmd for command in commands)

	def invoke(self, commands: list[ExclamCommand]) -> list[ExclamEdit]:
		for command in commands:
			if command.cmd == self.cmd:
				return command.fn(self.ctx, self.args)
		else:
			return []

	def removal(self) -> ExclamEdit:
		end_idx = self.ctx.flat_end_index()
		return ExclamEdit(end_idx - self.cmd_length, end_idx, "")
