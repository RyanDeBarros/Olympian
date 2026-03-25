from typing import override

from editor.core import REPLCommand, ProgramState
from editor.core.context import EditorContext


class BufListOpenCommand(REPLCommand):
	def __init__(self):
		super().__init__("buf.list-open")

	@override
	def execute(self):
		program = ProgramState.instance()
		if len(program.args) == 0:
			for bp in EditorContext.data_buffers():
				print(bp.asset_path)
			else:
				print("No buffers are currently open")
		else:
			self.print_arg_error("Expected 0 arguments")

	@override
	def help(self):
		print("help not implemented")  # DOC


def register():
	ProgramState.instance().machine.default().add_command(BufListOpenCommand())
