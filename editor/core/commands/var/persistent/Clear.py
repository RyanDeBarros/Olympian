from typing import override

from editor.core.REPL import REPLCommand, ProgramState
from .. import Storage


class VarPersistentClearCommand(REPLCommand):
	def __init__(self, program: ProgramState):
		super().__init__(program, "var.persistent.clear")

	@override
	def execute(self):
		if len(self.program.args) == 0:
			Storage.clear_persistent()
		else:
			self.print_arg_error("Expected 0 arguments")

	@override
	def help(self):
		print("help not implemented")  # DOC


def register(program: ProgramState):
	program.machine.default().add_command(VarPersistentClearCommand(program))
