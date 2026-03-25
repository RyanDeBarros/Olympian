from pathlib import Path
from typing import override

from editor.core import REPLCommand, ProgramState
from editor.core.buffers import BufferPath
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
		asset = Path(arg).resolve()
		if not asset.exists():
			self.print_arg_error(f"{PathUtils.printed_path(asset)} does not exist")
			return

		bp = BufferPath.from_asset(asset)
		# TODO v7 actually create buffer file if it doesn't exist
		if bp.buffer_path.exists():
			EditorContext.reveal_in_explorer(bp.buffer_path)
		else:
			print(f"{PathUtils.printed_path(asset)} is closed")


def register():
	ProgramState.instance().machine.default().add_command(BufRevealCommand())
