from typing import Callable


class ExclamCommand:
	def __init__(self, cmd: str, fn: Callable, info: str):
		self.cmd = cmd
		self.fn = fn
		self.info = info
		self.exclam = f"!{cmd}"
