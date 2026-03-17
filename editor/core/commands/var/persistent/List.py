from typing import override, Iterable

from prompt_toolkit.completion import CompleteEvent, Completion
from prompt_toolkit.document import Document

from editor.core import REPLCommand, ProgramState
from .. import Storage, VarCompleter


class VarPersistentListCommand(REPLCommand):
	def __init__(self, program: ProgramState):
		super().__init__(program, "var.persistent.list")

	@override
	def execute(self):
		if len(self.program.args) == 0:
			self.print_list("")
		elif len(self.program.args) == 1:
			self.print_list(self.program.args[0])
		else:
			self.print_arg_error("Expected 0-1 arguments")

	@staticmethod
	def print_list(prefix: str):
		keys = filter(lambda key: prefix in key, Storage.persistent_keys())
		for key in sorted(keys):
			print(key, '=', Storage.get_persistent(key))

	@override
	def help(self):
		print("help not implemented")  # DOC

	@override
	def get_completions(self, document: Document, complete_event: CompleteEvent) -> Iterable[Completion]:
		yield from VarCompleter.get_persistent_completions(document)


def register(program: ProgramState):
	program.machine.default().add_command(VarPersistentListCommand(program))
