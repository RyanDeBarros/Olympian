from typing import override, Iterable

from prompt_toolkit.completion import CompleteEvent, Completion
from prompt_toolkit.document import Document

from editor.core import REPLCommand, ProgramState
from . import Storage, VarCompleter


class VarDelCommand(REPLCommand):
	def __init__(self, program: ProgramState):
		super().__init__(program, "var.del")

	@override
	def execute(self):
		if len(self.program.args) == 0:
			self.print_arg_error("Expected at least 1 argument")
		else:
			for arg in self.program.args:
				Storage.del_temp(arg)

	@override
	def help(self):
		print("help not implemented")  # DOC

	@override
	def get_completions(self, document: Document, complete_event: CompleteEvent) -> Iterable[Completion]:
		yield from VarCompleter.get_temp_completions(document)


def register(program: ProgramState):
	program.machine.default().add_command(VarDelCommand(program))
