from typing import override, Iterable

from prompt_toolkit.completion import CompleteEvent, Completion
from prompt_toolkit.document import Document

from editor.core import REPLCommand, ProgramState, KeyCompleter


class VarPersistentListCommand(REPLCommand):
	def __init__(self):
		super().__init__("var.persistent.list")

	@override
	def execute(self):
		program = ProgramState.instance()
		if len(program.args) == 0:
			self.print_list("")
		elif len(program.args) == 1:
			self.print_list(program.args[0])
		else:
			self.print_arg_error("Expected 0-1 arguments")

	@staticmethod
	def print_list(prefix: str):
		persistent = ProgramState.instance().macros.persistent
		keys = filter(lambda key: prefix in key, persistent.keys())
		for key in sorted(keys):
			print(f"${key} = {persistent.get(key)}")

	@override
	def help(self):
		print("help not implemented")  # DOC

	@override
	def get_completions(self, document: Document, complete_event: CompleteEvent) -> Iterable[Completion]:
		yield from KeyCompleter.get_keys_completions(document, ProgramState.instance().macros.persistent.keys())


def register():
	ProgramState.instance().machine.default().add_command(VarPersistentListCommand())
