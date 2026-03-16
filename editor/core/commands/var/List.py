from typing import override

from editor.core.REPL import REPLCommand, REPLStateMachine, ProgramState
from editor.tools import eprint
from . import Storage


class VarListCommand(REPLCommand):
	def __init__(self):
		super().__init__("var.list")

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
		keys = filter(lambda key: prefix in key, Storage.temp_keys())
		for key in sorted(keys):
			print(key)


def register(machine: REPLStateMachine):
	machine.default.add_command(VarListCommand())

# TODO v7 var.publish command to save key to persistent storage, and var.unpublish to remove key from persistent storage. No other way to get/set/del from persistent storage.
# TODO v7 var.persistent <key> for persistent listing
