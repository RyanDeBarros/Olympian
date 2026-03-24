from typing import override

from editor.core import REPLCommand, ProgramState
from editor.core.context import EditorContext, PathUtils


class EditorMacrosPathCommand(REPLCommand):
	def __init__(self, program: ProgramState):
		super().__init__(program, "editor.macros.path")

	@override
	def execute(self):
		EditorContext.assert_initialized(self.program.project_dir)

		if len(self.program.args) == 0:
			print(PathUtils.printed_path(self.program.macros.persistent_path()))
		else:
			self.print_arg_error("Expected 0 arguments")

	@override
	def help(self):
		print("help not implemented")  # DOC


def register(program: ProgramState):
	program.machine.default().add_command(EditorMacrosPathCommand(program))
