from typing import override, Iterable

from prompt_toolkit.completion import CompleteEvent, Completion
from prompt_toolkit.document import Document

from editor.core import REPLCommand, ProgramState, KeyCompleter


class VarListCommand(REPLCommand):
	def __init__(self):
		super().__init__("var.list")

	@override
	def requires_initialized_editor(self):
		return False

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
		temporary = ProgramState.instance().macros.temporary
		keys = filter(lambda key: prefix in key, temporary.keys())
		for key in sorted(keys):
			print(f"${key} = {temporary.get(key)}")

	@override
	def help(self):
		print("help not implemented")  # DOC

	@override
	def get_completions(self, document: Document, complete_event: CompleteEvent) -> Iterable[Completion]:
		yield from KeyCompleter.get_keys_completions(document, ProgramState.instance().macros.temporary.keys())


def register():
	ProgramState.instance().machine.default().add_command(VarListCommand())
