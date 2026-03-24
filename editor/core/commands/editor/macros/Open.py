from typing import override

from editor.core import REPLCommand, ProgramState
from editor.core.context import EditorContext


class EditorMacrosOpenCommand(REPLCommand):
	def __init__(self):
		super().__init__("editor.macros.open")

	@override
	def execute(self):
		program = ProgramState.instance()
		if len(program.args) == 0:
			EditorContext.open_with_default_app(program.macros.persistent_path)
		else:
			self.print_arg_error("Expected 0 arguments")

	@override
	def help(self):
		print("help not implemented")  # DOC


def register():
	ProgramState.instance().machine.default().add_command(EditorMacrosOpenCommand())
