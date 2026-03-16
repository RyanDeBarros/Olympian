from typing import override

from editor.core import Resolver
from editor.core.REPL import REPLCommand, ProgramState
from editor.tools import eprint
from . import Storage


class VarSetCommand(REPLCommand):
	def __init__(self, program: ProgramState):
		super().__init__(program, "var.set")

	@override
	def execute(self):
		if len(self.program.args) == 2:
			key = self.program.args[0]
			value = self.program.args[1]
			if Resolver.is_valid_macro_key(key):
				Storage.set_temp(key, value)
			else:
				eprint(f"Key must only contain alphanumeric characters, '-', or '_'")
		else:
			eprint("Expected 2 arguments")

	@override
	def help(self):
		print("help not implemented")  # DOC

	@override
	def expand_macros(self):
		return False


def register(program: ProgramState):
	program.machine.default().add_command(VarSetCommand(program))
