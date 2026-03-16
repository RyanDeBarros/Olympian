from typing import override

from editor.core.REPL import REPLCommand, REPLStateMachine, ProgramState


class ProjectNewCommand(REPLCommand):
	def __init__(self):
		super().__init__("project.new")  # TODO v7 autocomplete should complete project first (don't display anything after .), then post-dot display the next possible commands

	@override
	def execute(self, program: ProgramState, args: list[str]):
		pass  # TODO v7


def register(machine: REPLStateMachine):
	machine.default.add_command(ProjectNewCommand())
