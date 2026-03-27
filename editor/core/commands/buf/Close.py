from typing import override

from editor.core import REPLCommand, ProgramState
from editor.core.context import PathUtils


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
		asset = self.resolve_asset_path(arg)
		if asset is None:
			return

		buffer = ProgramState.instance().get_buffer(asset)
		if buffer is None:
			print(f"{PathUtils.printed_path(asset)} is not open")
		elif buffer.close():
			ProgramState.instance().buffers.remove(buffer)


def register():
	ProgramState.instance().machine.default().add_command(BufCloseCommand())
