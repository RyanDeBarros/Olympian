from typing import override

from editor.core import REPLCommand, ProgramState


class BufCloseCommand(REPLCommand):
	def __init__(self):
		super().__init__("buf.close")

	@override
	def execute(self):
		program = ProgramState.instance()
		if len(program.args) == 0:
			self.print_arg_error("Expected at least 1 argument")
		else:
			for arg in program.args:
				self.close(arg)

	@override
	def help(self):
		print("help not implemented")  # DOC

	def close(self, arg: str):
		pass  # TODO v7


def register():
	ProgramState.instance().machine.default().add_command(BufCloseCommand())
