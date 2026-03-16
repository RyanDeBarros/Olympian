from typing import override

from editor.core.REPL import REPLCommand, REPLStateMachine, ProgramState
from editor.tools import eprint
from . import Storage


class TempGetCommand(REPLCommand):
	def __init__(self):
		super().__init__("temp.get")

	@override
	def execute(self, program: ProgramState):
		if len(program.args) == 1:
			try:
				print(Storage.get_temp(program.args[0]))  # TODO v7 expand recursive macros
			except KeyError:
				eprint("Key does not exist")
		else:
			eprint("Expected 1 argument")


def register(machine: REPLStateMachine):
	machine.default.add_command(TempGetCommand())
