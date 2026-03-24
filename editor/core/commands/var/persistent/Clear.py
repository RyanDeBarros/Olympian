from typing import override

from editor.core import REPLCommand, ProgramState
from editor.core.context import EditorContext


class VarPersistentClearCommand(REPLCommand):
	def __init__(self):
		super().__init__("var.persistent.clear")

	@override
	def execute(self):
		EditorContext.assert_initialized()

		program = ProgramState.instance()
		if len(program.args) == 0:
			program.macros.persistent.clear()
		else:
			self.print_arg_error("Expected 0 arguments")

	@override
	def help(self):
		print("help not implemented")  # DOC


def register():
	ProgramState.instance().machine.default().add_command(VarPersistentClearCommand())
