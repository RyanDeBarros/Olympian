from typing import override

from editor.core import REPLCommand, ProgramState


class VarClearCommand(REPLCommand):
	def __init__(self):
		super().__init__("var.clear")

	@override
	def requires_initialized_editor(self):
		return False

	@override
	def execute(self):
		program = ProgramState.instance()
		if len(program.args) == 0:
			program.macros.temporary.clear()
		else:
			self.print_arg_error("Expected 0 arguments")

	@override
	def help(self):
		print("help not implemented")  # DOC


def register():
	ProgramState.instance().machine.default().add_command(VarClearCommand())
