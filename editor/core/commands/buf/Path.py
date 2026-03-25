from pathlib import Path
from typing import override

from editor.core import REPLCommand, ProgramState
from editor.core.buffers import BufferPath
from editor.core.context import EditorContext, PathUtils


class BufPathCommand(REPLCommand):
	def __init__(self):
		super().__init__("buf.path")

	@override
	def execute(self):
		program = ProgramState.instance()
		if len(program.args) == 0:
			self.print_arg_error("Expected at least 1 argument")
		else:
			for arg in program.args:
				self.path(arg)

	@override
	def help(self):
		print("help not implemented")  # DOC

	def path(self, arg: str):
		asset = Path(arg).resolve()
		if not asset.exists():
			self.print_arg_error(f"{PathUtils.printed_path(asset)} does not exist")
			return

		bp = BufferPath.from_asset(asset)
		print(PathUtils.printed_path(bp.buffer_path), f"({'open' if bp.buffer_path.exists() else 'closed'})")

def register():
	ProgramState.instance().machine.default().add_command(BufPathCommand())
