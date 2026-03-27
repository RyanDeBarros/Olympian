from pathlib import Path
from typing import override

from editor.core import REPLCommand, ProgramState
from editor.core.buffers import BufferChooser
from editor.core.context import PathUtils, EditorContext


class BufRevealCommand(REPLCommand):
	def __init__(self):
		super().__init__("buf.reveal")

	@override
	def execute(self):
		program = ProgramState.instance()
		if len(program.args) == 0:
			self.print_arg_error("Expected at least 1 argument")
		else:
			for arg in program.args:
				self.reveal(arg)

	@override
	def help(self):
		print("help not implemented")  # DOC

	def reveal(self, arg: str):
		asset = self.resolve_asset_path(arg)
		if asset is None:
			return

		buffer = ProgramState.instance().get_buffer(asset)
		if buffer is not None:
			EditorContext.open_with_default_app(buffer.buf.buffer_path)
		else:
			buffer = BufferChooser.instance().buffer(asset)
			if buffer is not None:
				ProgramState.instance().buffers.append(buffer)
				buffer.open()
				EditorContext.reveal_in_explorer(buffer.buf.buffer_path)
			else:
				self.print_arg_error(f"{PathUtils.printed_path(asset)} asset type is not supported")



def register():
	ProgramState.instance().machine.default().add_command(BufRevealCommand())
