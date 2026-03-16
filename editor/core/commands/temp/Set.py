from typing import override

from editor.core import Resolver
from editor.core.REPL import REPLCommand, REPLStateMachine, ProgramState
from editor.tools import eprint
from . import Storage


class TempSetCommand(REPLCommand):
	def __init__(self):
		super().__init__("temp.set")

	@override
	def execute(self, program: ProgramState):
		if len(program.args) == 2:
			key = program.args[0]
			value = program.args[1]
			if Resolver.is_valid_macro_key(key):
				Storage.set_temp(key, value)
			else:
				eprint(f"Key must only contain alphanumeric characters, '-', or '_'")
		else:
			eprint("Expected 2 arguments")

	def expand_macros(self):
		return False


def register(machine: REPLStateMachine):
	machine.default.add_command(TempSetCommand())


# TODO v7 temp.list <var> list commands that start with <var>, or pass nothing and list all commands. temp.persistent <var> for persistent listing
# TODO v7 temp.publish command to save var to persistent storage, and temp.unpublish to remove var from persistent storage. No other way to get/set/del from persistent storage.
# TODO v7 rename temp command to var
