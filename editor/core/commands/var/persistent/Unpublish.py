from typing import override

from editor.core.REPL import REPLCommand, REPLStateMachine, ProgramState
from editor.tools import eprint
from .. import Storage


class VarPersistentUnpublishCommand(REPLCommand):
	def __init__(self):
		super().__init__("var.persistent.unpublish")

	@override
	def execute(self, program: ProgramState):
		if len(program.args) == 1:
			Storage.del_persistent(program.args[0])
		else:
			eprint("Expected 1 argument")


def register(machine: REPLStateMachine):
	machine.default.add_command(VarPersistentUnpublishCommand())
