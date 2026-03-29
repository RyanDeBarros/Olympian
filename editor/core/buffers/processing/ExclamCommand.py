from typing import Callable, TYPE_CHECKING

if TYPE_CHECKING:
	from .BufferSection import BufferSectionContext


# TODO v7.1 allow for passing arguments to fn, such as `!new-slot(count=3)`
class ExclamCommand:
	def __init__(self, cmd: str, fn: Callable[["BufferSectionContext"], None], info: str):
		self.cmd = cmd
		self.fn = fn
		self.info = info
		self.exclam = f"!{cmd}"
