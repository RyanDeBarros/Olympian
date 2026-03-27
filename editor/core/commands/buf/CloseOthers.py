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
			safe = [self.resolve_asset_path(arg) for arg in program.args]
			keep = []
			for buffer in ProgramState.instance().buffers:
				if buffer.buf.asset_path in safe or not buffer.close():
					keep.append(buffer)
			ProgramState.instance().buffers = keep

	@override
	def help(self):
		print("help not implemented")  # DOC


def register():
	ProgramState.instance().machine.default().add_command(BufCloseOthersCommand())
