from typing import override

from editor.core import REPLCommand, ProgramState


class ExitCommand(REPLCommand):
	def __init__(self):
		super().__init__("exit")

	@override
	def requires_initialized_editor(self):
		return False

	@override
	def execute(self):
		ProgramState.instance().exit = True

	@override
	def help(self):
		print("help not implemented")  # DOC


def register():
	ProgramState.instance().machine.default().add_command(ExitCommand())
