from typing import override

from editor.core import REPLCommand, ProgramState
from editor.core.context import EditorContext


class BufCloseAllCommand(REPLCommand):
	def __init__(self):
		super().__init__("buf.close-all")

	@override
	def execute(self):
		program = ProgramState.instance()
		EditorContext.assert_initialized(program.project_dir)

		if len(program.args) == 0:
			pass  # TODO v7
		else:
			self.print_arg_error("Expected 0 arguments")

	@override
	def help(self):
		print("help not implemented")  # DOC


def register():
	ProgramState.instance().machine.default().add_command(BufCloseAllCommand())
