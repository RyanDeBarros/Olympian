from typing import override

from editor.core import Resolver
from editor.core.REPL import REPLCommand, REPLStateMachine, ProgramState
from editor.tools import eprint
from . import Storage


class VarGetCommand(REPLCommand):
	def __init__(self):
		super().__init__("var.get")

	@override
	def execute(self, program: ProgramState):
		if len(program.args) == 1:
			try:
				value = Storage.get_temp(program.args[0])
				print("Value:", value)
				expanded = Resolver.expand_macros(value)
				if expanded != value:
					print("Expanded:", value)
			except KeyError:
				eprint("Key does not exist")
		else:
			eprint("Expected 1 argument")


def register(machine: REPLStateMachine):
	machine.default.add_command(VarGetCommand())
