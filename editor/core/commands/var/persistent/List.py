from typing import override

from editor.core.REPL import REPLCommand, REPLStateMachine, ProgramState
from editor.tools import eprint
from .. import Storage


class VarPersistentListCommand(REPLCommand):
	def __init__(self):
		super().__init__("var.persistent.list")

	@override
	def execute(self, program: ProgramState):
		if len(program.args) == 0:
			self.print_list("")
		elif len(program.args) == 1:
			self.print_list(program.args[0])
		else:
			eprint("Expected 0-1 arguments")

	@staticmethod
	def print_list(prefix: str):
		keys = filter(lambda key: prefix in key, Storage.persistent_keys())
		for key in sorted(keys):
			print(key, '=', Storage.get_persistent(key))


def register(machine: REPLStateMachine):
	machine.default.add_command(VarPersistentListCommand())
