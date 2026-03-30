from typing import Callable, TYPE_CHECKING

from . import ExclamEdit

if TYPE_CHECKING:
	from . import ExclamContext


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


class ExclamCommand:
	def __init__(self, cmd: str, fn: Callable[["ExclamContext", ExclamArgs], list[ExclamEdit]], info: str):
		self.cmd = cmd
		self.fn = fn
		self.info = info
