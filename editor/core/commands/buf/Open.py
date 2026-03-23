from typing import override

from editor.core import REPLCommand, ProgramState


class BufOpenCommand(REPLCommand):
	def __init__(self, program: ProgramState):
		super().__init__(program, "buf.open")

	@override
	def execute(self):
		if len(self.program.args) == 0:
			self.print_arg_error("Expected at least 1 argument")
		else:
			for arg in self.program.args:
				self.open(arg)

	@override
	def help(self):
		print("help not implemented")  # DOC

	def open(self, arg: str):
		pass  # TODO v7


def register(program: ProgramState):
	program.machine.default().add_command(BufOpenCommand(program))
