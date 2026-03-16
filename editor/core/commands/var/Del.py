from typing import override

from editor.core.REPL import REPLCommand, ProgramState
from editor.tools import eprint
from . import Storage


class VarDelCommand(REPLCommand):
	def __init__(self, program: ProgramState):
		super().__init__(program, "var.del")

	@override
	def execute(self):
		if len(self.program.args) == 1:
			Storage.del_temp(self.program.args[0])
		else:
			eprint("Expected 1 argument")

	@override
	def help(self):
		print("help not implemented")  # DOC


def register(program: ProgramState):
	program.machine.default().add_command(VarDelCommand(program))
