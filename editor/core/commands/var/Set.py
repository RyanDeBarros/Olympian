from typing import override

from editor.core import Resolver
from editor.core.REPL import REPLCommand, ProgramState
from . import Storage


class VarSetCommand(REPLCommand):
	def __init__(self, program: ProgramState):
		super().__init__(program, "var.set")

	@override
	def execute(self):
		if len(self.program.args) > 0 and len(self.program.args) & 1 == 0:
			for i in range(len(self.program.args) // 2):
				key = self.program.args[2 * i]
				value = self.program.args[2 * i + 1]
				if Resolver.is_valid_macro_key(key):
					Storage.set_temp(key, value)
				else:
					self.print_arg_error(f"${key} must only contain alphanumeric characters, '-', or '_'")
		else:
			self.print_arg_error("Expected a non-zero even number of arguments")

	@override
	def help(self):
		print("help not implemented")  # DOC

	@override
	def expand_macros(self):
		return False


def register(program: ProgramState):
	program.machine.default().add_command(VarSetCommand(program))
