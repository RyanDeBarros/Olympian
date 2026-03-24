from typing import override

from editor.core import REPLCommand, ProgramState
from editor.core.context import EditorContext


class EditorInitCommand(REPLCommand):
	def __init__(self, program: ProgramState):
		super().__init__(program, "editor.init")

	@override
	def execute(self):
		if len(self.program.args) == 0:
			if EditorContext.is_initialized(self.program.project_dir):
				print("Editor is already initialized")
			else:
				EditorContext.initialize(self.program)
		else:
			self.print_arg_error("Expected 0 arguments")

	@override
	def help(self):
		print("help not implemented")  # DOC


def register(program: ProgramState):
	program.machine.default().add_command(EditorInitCommand(program))
