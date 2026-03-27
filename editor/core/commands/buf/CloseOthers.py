from typing import override

from editor.core import REPLCommand, ProgramState


class BufCloseOthersCommand(REPLCommand):
	def __init__(self):
		super().__init__("buf.close-others")

	@override
	def execute(self):
		program = ProgramState.instance()
		if len(program.args) == 0:
			self.print_arg_error("Expected at least 1 argument")
		else:
			pass  # TODO v7.1

	@override
	def help(self):
		print("help not implemented")  # DOC


def register():
	ProgramState.instance().machine.default().add_command(BufCloseOthersCommand())
