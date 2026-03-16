from typing import override

from editor.core import Resolver
from editor.core.REPL import REPLCommand, ProgramState
from .. import Storage


class VarPersistentViewCommand(REPLCommand):
	def __init__(self, program: ProgramState):
		super().__init__(program, "var.persistent.view")

	@override
	def execute(self):
		if len(self.program.args) == 1:
			try:
				value = Storage.get_persistent(self.program.args[0])
				print(value)
				expanded = Resolver.expand_macros(value)
				if expanded != value:
					print("Expanded:", value)
			except KeyError:
				self.print_arg_error("Key does not exist")
		else:
			self.print_arg_error("Expected 1 argument")

	@override
	def help(self):
		print("help not implemented")  # DOC


def register(program: ProgramState):
	program.machine.default().add_command(VarPersistentViewCommand(program))
