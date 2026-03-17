from typing import override

from editor.core.REPL import REPLCommand, ProgramState
from editor.core.commands.var import Storage


class VarPersistentLoadCommand(REPLCommand):
	def __init__(self, program: ProgramState):
		super().__init__(program, "var.persistent.load")

	@override
	def execute(self):
		if len(self.program.args) == 0:
			for key in Storage.persistent_keys():
				self.load(key)
		else:
			for arg in self.program.args:
				self.load(arg)

	def load(self, key: str):
		try:
			value = Storage.get_persistent(key)
		except KeyError:
			self.print_arg_error(f"${key} is not a persistent var")
		else:
			Storage.set_temp(key, value)

	@override
	def help(self):
		print("help not implemented")  # DOC


def register(program: ProgramState):
	program.machine.default().add_command(VarPersistentLoadCommand(program))
