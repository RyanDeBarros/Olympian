from pathlib import Path
from typing import override

from editor.core import REPLCommand, ProgramState
from editor.core.buffers import BufferChooser
from editor.core.context import PathUtils


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
		asset = Path(arg).resolve()
		if not asset.exists():
			self.print_arg_error(f"{PathUtils.printed_path(asset)} does not exist")
			return

		buffer = ProgramState.instance().get_buffer(asset)
		if buffer is not None:
			print(f"{PathUtils.printed_path(asset)} is already open")
		else:
			buffer = BufferChooser.instance().buffer(asset)
			if buffer is not None:
				ProgramState.instance().buffers.append(buffer)
				buffer.open()
			else:
				self.print_arg_error(f"{PathUtils.printed_path(asset)} asset type is not supported")


def register():
	ProgramState.instance().machine.default().add_command(BufEditCommand())
