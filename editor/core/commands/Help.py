from typing import override

from editor.core.REPL import REPLCommand, REPLStateMachine, ProgramState


class HelpCommand(REPLCommand):
	def __init__(self):
		super().__init__("help")

	@override
	def execute(self, program: ProgramState, args: list[str]):
		pass  # TODO v7


def register(machine: REPLStateMachine):
	machine.default.add_command(HelpCommand())
