from typing import override

from editor.core import REPLCommand, ProgramState
from editor.core.context import EditorContext


# TODO v7 remove editor.init once project.create is finished, since editor context might require project oly file at project dir root.
class EditorInitCommand(REPLCommand):
	def __init__(self):
		super().__init__("editor.init")

	@override
	def requires_initialized_editor(self):
		return False

	@override
	def execute(self):
		program = ProgramState.instance()
		if len(program.args) == 0:
			if EditorContext.is_initialized():
				print("Editor is already initialized")
			else:
				EditorContext.initialize()
		else:
			self.print_arg_error("Expected 0 arguments")

	@override
	def help(self):
		print("help not implemented")  # DOC


def register():
	ProgramState.instance().machine.default().add_command(EditorInitCommand())
