from abc import ABC, abstractmethod
from typing import Iterable

from prompt_toolkit.completion import CompleteEvent, Completion
from prompt_toolkit.document import Document

from editor.tools import eprint
from . import PathCompleter


class REPLCommand(ABC):
	from . import ProgramState
	def __init__(self, program: ProgramState, name: str):
		self.program = program
		self.name = name

	@abstractmethod
	def execute(self):
		raise NotImplementedError()

	@abstractmethod
	def help(self):
		raise NotImplementedError()

	def expand_macros(self):
		return True

	def get_completions(self, document: Document, complete_event: CompleteEvent) -> Iterable[Completion]:
		yield from PathCompleter.get_path_completions(document)

	def print_arg_error(self, msg: str):
		eprint(f"{msg}. Run `help {self.name}` for more details")
