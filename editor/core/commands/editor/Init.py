from typing import override

from editor.core import REPLCommand, ProgramState
from editor.core.context import EditorContext


class EditorInitCommand(REPLCommand):
	def __init__(self):
		super().__init__("editor.init")

	@override
	def execute(self):
		program = ProgramState.instance()
		if len(program.args) == 0:
			if EditorContext.is_initialized(program.project_dir):
				print("Editor is already initialized")
			else:
				EditorContext.initialize(program)
		else:
			self.print_arg_error("Expected 0 arguments")

	@override
	def help(self):
		print("help not implemented")  # DOC


def register():
	ProgramState.instance().machine.default().add_command(EditorInitCommand())
