from typing import override

from editor.core.REPL import REPLCommand, REPLStateMachine, ProgramState


class HelpCommand(REPLCommand):
	def __init__(self):
		super().__init__("help")

	@override
	def execute(self, program: ProgramState):
		pass  # TODO v7 general info, or take second argument that is a command (use custom autocomplete for second argument) to get info on the command. Define help() abstract method for REPLCommand.


def register(machine: REPLStateMachine):
	machine.default.add_command(HelpCommand())
