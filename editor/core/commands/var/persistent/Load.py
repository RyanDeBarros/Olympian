from typing import override

from editor.core.REPL import REPLCommand, ProgramState
from editor.core.commands.var import Storage
from editor.tools import eprint


class VarPersistentLoadCommand(REPLCommand):
	def __init__(self, program: ProgramState):
		super().__init__(program, "var.persistent.load")

	@override
	def execute(self):
		if len(self.program.args) == 0:
			for key in Storage.persistent_keys():
				self.load(key)
		elif len(self.program.args) == 1:
			self.load(self.program.args[0])
		else:
			eprint("Expected 0-1 arguments")

	@staticmethod
	def load(key: str):
		value = Storage.get_persistent(key)
		Storage.set_temp(key, value)

	@override
	def help(self):
		print("help not implemented")  # DOC


def register(program: ProgramState):
	program.machine.default.add_command(VarPersistentLoadCommand(program))
