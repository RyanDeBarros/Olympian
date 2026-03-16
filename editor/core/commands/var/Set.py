from typing import override

from editor.core import Resolver
from editor.core.REPL import REPLCommand, REPLStateMachine, ProgramState
from editor.tools import eprint
from . import Storage


class VarSetCommand(REPLCommand):
	def __init__(self):
		super().__init__("var.set")

	@override
	def execute(self, program: ProgramState):
		if len(program.args) == 2:
			key = program.args[0]
			value = program.args[1]
			if Resolver.is_valid_macro_key(key):
				Storage.set_temp(key, value)
			else:
				eprint(f"Key must only contain alphanumeric characters, '-', or '_'")
		else:
			eprint("Expected 2 arguments")

	def expand_macros(self):
		return False


def register(machine: REPLStateMachine):
	machine.default.add_command(VarSetCommand())
