from typing import override

from editor.core.REPL import REPLCommand, REPLStateMachine, ProgramState
from editor.tools import eprint
from . import Storage


class TempSetCommand(REPLCommand):
	def __init__(self):
		super().__init__("temp.set")

	@override
	def execute(self, program: ProgramState):
		if len(program.args) == 2:
			# TODO v7 import GROUP_OPEN, GROUP_CLOSE instead of raw char. Also add MACRO_PREFIX for $.
			key = program.args[0]
			if '[' in key or ']' in key or '$' in key:
				eprint("Invalid key: [, ], and $ are not allowed in name")
			else:
				Storage.set_temp(program.args[0], program.args[1])
		else:
			eprint("Expected 2 arguments")


def register(machine: REPLStateMachine):
	machine.default.add_command(TempSetCommand())
