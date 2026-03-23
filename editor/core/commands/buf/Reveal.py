from typing import override

from editor.core import REPLCommand, ProgramState
from editor.core.context import EditorContext


class BufRevealCommand(REPLCommand):
	def __init__(self, program: ProgramState):
		super().__init__(program, "buf.reveal")

	@override
	def execute(self):
		EditorContext.assert_initialized(self.program.project_dir)

		if len(self.program.args) == 0:
			self.print_arg_error("Expected at least 1 argument")
		else:
			for arg in self.program.args:
				self.reveal(arg)

	@override
	def help(self):
		print("help not implemented")  # DOC

	def reveal(self, arg: str):
		pass  # TODO v7


def register(program: ProgramState):
	program.machine.default().add_command(BufRevealCommand(program))
