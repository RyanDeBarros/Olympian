from typing import override

from editor.core.REPL import REPLCommand, REPLStateMachine, ProgramState
from editor.tools import eprint
from .. import Storage


class VarPersistentPublishCommand(REPLCommand):
	def __init__(self):
		super().__init__("var.persistent.publish")

	@override
	def execute(self, program: ProgramState):
		# TODO v7 here and in other var commands, accept multiple arguments to execute simultaneously. e.g. here publish multiple variables, and add autocomplete for them
		if len(program.args) == 1:
			key = program.args[0]
			try:
				value = Storage.get_temp(key)
			except KeyError:
				eprint("Key does not exist")
			else:
				Storage.set_persistent(key, value)
		else:
			eprint("Expected 1 argument")  # TODO v7 in error messages, be more descriptive about the expected syntax.


def register(machine: REPLStateMachine):
	machine.default.add_command(VarPersistentPublishCommand())
