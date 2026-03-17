from typing import override

from editor.core import REPLCommand, ProgramState
from . import Storage


class VarListCommand(REPLCommand):
	def __init__(self, program: ProgramState):
		super().__init__(program, "var.list")

	@override
	def execute(self):
		if len(self.program.args) == 0:
			self.print_list("")
		elif len(self.program.args) == 1:
			self.print_list(self.program.args[0])
		else:
			self.print_arg_error("Expected 0-1 arguments")

	@staticmethod
	def print_list(prefix: str):
		keys = filter(lambda key: prefix in key, Storage.temp_keys())
		for key in sorted(keys):
			print(key, '=', Storage.get_temp(key))

	@override
	def help(self):
		print("help not implemented")  # DOC


def register(program: ProgramState):
	program.machine.default().add_command(VarListCommand(program))
