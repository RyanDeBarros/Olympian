from typing import override

from editor.core import REPLCommand, ProgramState


class BufEditCommand(REPLCommand):
	def __init__(self):
		super().__init__("buf.edit")

	@override
	def execute(self):
		program = ProgramState.instance()
		if len(program.args) == 0:
			self.print_arg_error("Expected at least 1 argument")
		else:
			for arg in program.args:
				self.edit(arg)

	@override
	def help(self):
		print("help not implemented")  # DOC

	def edit(self, arg: str):
		pass  # TODO v7 actually create buffer file if it doesn't exist


def register():
	ProgramState.instance().machine.default().add_command(BufEditCommand())
