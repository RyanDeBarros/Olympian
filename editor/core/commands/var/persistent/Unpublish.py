from typing import override, Iterable

from prompt_toolkit.completion import CompleteEvent, Completion
from prompt_toolkit.document import Document

from editor.core import REPLCommand, ProgramState, KeyCompleter
from editor.core.context import EditorContext


class VarPersistentUnpublishCommand(REPLCommand):
	def __init__(self):
		super().__init__("var.persistent.unpublish")

	@override
	def execute(self):
		EditorContext.assert_initialized()

		program = ProgramState.instance()
		if len(program.args) == 0:
			self.print_arg_error("Expected at least 1 argument")
		else:
			for arg in program.args:
				program.macros.persistent.remove(arg)

	@override
	def help(self):
		print("help not implemented")  # DOC

	@override
	def get_completions(self, document: Document, complete_event: CompleteEvent) -> Iterable[Completion]:
		yield from KeyCompleter.get_keys_completions(document, ProgramState.instance().macros.persistent.keys())


def register():
	ProgramState.instance().machine.default().add_command(VarPersistentUnpublishCommand())
