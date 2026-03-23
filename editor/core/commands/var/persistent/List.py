from typing import override, Iterable

from prompt_toolkit.completion import CompleteEvent, Completion
from prompt_toolkit.document import Document

from editor.core import REPLCommand, ProgramState, KeyCompleter
from editor.core.context import EditorContext


class VarPersistentListCommand(REPLCommand):
	def __init__(self, program: ProgramState):
		super().__init__(program, "var.persistent.list")

	@override
	def execute(self):
		EditorContext.assert_initialized(self.program.project_dir)

		if len(self.program.args) == 0:
			self.print_list("")
		elif len(self.program.args) == 1:
			self.print_list(self.program.args[0])
		else:
			self.print_arg_error("Expected 0-1 arguments")

	def print_list(self, prefix: str):
		keys = filter(lambda key: prefix in key, self.program.macros.persistent.keys())
		for key in sorted(keys):
			print(f"${key} = {self.program.macros.persistent.get(key)}")

	@override
	def help(self):
		print("help not implemented")  # DOC

	@override
	def get_completions(self, document: Document, complete_event: CompleteEvent) -> Iterable[Completion]:
		yield from KeyCompleter.get_keys_completions(document, self.program.macros.persistent.keys())


def register(program: ProgramState):
	program.machine.default().add_command(VarPersistentListCommand(program))
