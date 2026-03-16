from typing import override

from editor.core.REPL import REPLCommand, REPLStateMachine, ProgramState


class ProjectCreateCommand(REPLCommand):
	def __init__(self):
		super().__init__("project.create")

	@override
	def execute(self, program: ProgramState, args: list[str]):
		pass  # TODO v7


def register(machine: REPLStateMachine):
	machine.default.add_command(ProjectCreateCommand())
