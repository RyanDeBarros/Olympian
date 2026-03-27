from typing import override

from editor.core import REPLCommand, ProgramState


class BufCloseAllCommand(REPLCommand):
	def __init__(self):
		super().__init__("buf.close-all")

	@override
	def execute(self):
		program = ProgramState.instance()
		if len(program.args) == 0:
			keep = []
			for buffer in ProgramState.instance().buffers:
				if not buffer.close():
					keep.append(buffer)
			ProgramState.instance().buffers = keep
		else:
			self.print_arg_error("Expected 0 arguments")

	@override
	def help(self):
		print("help not implemented")  # DOC


def register():
	ProgramState.instance().machine.default().add_command(BufCloseAllCommand())
