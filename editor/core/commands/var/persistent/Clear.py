from typing import override

from editor.core.REPL import REPLCommand, REPLStateMachine, ProgramState
from editor.tools import eprint
from .. import Storage


class VarPersistentClearCommand(REPLCommand):
	def __init__(self):
		super().__init__("var.persistent.clear")

	@override
	def execute(self, program: ProgramState):
		if len(program.args) == 0:
			Storage.clear_persistent()
		else:
			eprint("Expected 0 arguments")


def register(machine: REPLStateMachine):
	machine.default.add_command(VarPersistentClearCommand())
