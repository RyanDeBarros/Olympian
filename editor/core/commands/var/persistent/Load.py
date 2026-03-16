from typing import override

from editor.core.REPL import REPLCommand, REPLStateMachine, ProgramState
from editor.core.commands.var import Storage
from editor.tools import eprint


class VarPersistentLoadCommand(REPLCommand):
	def __init__(self):
		super().__init__("var.persistent.load")

	@override
	def execute(self, program: ProgramState):
		if len(program.args) == 0:
			for key in Storage.persistent_keys():
				self.load(key)
		elif len(program.args) == 1:
			self.load(program.args[0])
		else:
			eprint("Expected 0-1 arguments")

	@staticmethod
	def load(key: str):
		value = Storage.get_persistent(key)
		Storage.set_temp(key, value)


def register(machine: REPLStateMachine):
	machine.default.add_command(VarPersistentLoadCommand())
