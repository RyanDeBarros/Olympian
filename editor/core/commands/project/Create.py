from typing import override

from editor.core import REPLCommand, ProgramState


class ProjectCreateCommand(REPLCommand):
	def __init__(self):
		super().__init__("project.create")

	@override
	def execute(self):
		pass  # TODO v7 create project files. also init editor

	@override
	def help(self):
		print("help not implemented")  # DOC


def register():
	ProgramState.instance().machine.default().add_command(ProjectCreateCommand())
