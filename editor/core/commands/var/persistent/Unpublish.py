from typing import override

from editor.core.REPL import REPLCommand, ProgramState
from .. import Storage


class VarPersistentUnpublishCommand(REPLCommand):
	def __init__(self, program: ProgramState):
		super().__init__(program, "var.persistent.unpublish")

	@override
	def execute(self):
		if len(self.program.args) == 0:
			self.print_arg_error("Expected at least 1 argument")
		else:
			for arg in self.program.args:
				Storage.del_persistent(arg)

	@override
	def help(self):
		print("help not implemented")  # DOC


def register(program: ProgramState):
	program.machine.default().add_command(VarPersistentUnpublishCommand(program))
