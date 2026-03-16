from typing import override

from editor.core.REPL import REPLCommand, REPLStateMachine, ProgramState


class ExitCommand(REPLCommand):
	def __init__(self):
		super().__init__("exit")

	@override
	def execute(self, program: ProgramState):
		program.exit = True


def register(machine: REPLStateMachine):
	machine.default.add_command(ExitCommand())
