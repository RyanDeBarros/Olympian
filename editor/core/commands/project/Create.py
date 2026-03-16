from typing import override

from editor.core.REPL import REPLCommand, ProgramState


class ProjectCreateCommand(REPLCommand):
	def __init__(self, program: ProgramState):
		super().__init__(program, "project.create")

	@override
	def execute(self):
		pass  # TODO v7

	@override
	def help(self):
		print("help not implemented")  # DOC


def register(program: ProgramState):
	program.machine.default.add_command(ProjectCreateCommand(program))
