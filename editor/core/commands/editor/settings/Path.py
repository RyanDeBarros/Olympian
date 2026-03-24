from typing import override

from editor.core import REPLCommand, ProgramState
from editor.core.context import PathUtils


class EditorSettingsPathCommand(REPLCommand):
	def __init__(self):
		super().__init__("editor.settings.path")

	@override
	def execute(self):
		program = ProgramState.instance()
		if len(program.args) == 0:
			print(PathUtils.printed_path(program.settings.persistent_path))
		else:
			self.print_arg_error("Expected 0 arguments")

	@override
	def help(self):
		print("help not implemented")  # DOC


def register():
	ProgramState.instance().machine.default().add_command(EditorSettingsPathCommand())
