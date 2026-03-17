from typing import override

from editor.core import REPLCommand, ProgramState


class ExitCommand(REPLCommand):
	def __init__(self, program: ProgramState):
		super().__init__(program, "exit")

	@override
	def execute(self):
		self.program.exit = True

	@override
	def help(self):
		print("help not implemented")  # DOC


def register(program: ProgramState):
	program.machine.default().add_command(ExitCommand(program))
