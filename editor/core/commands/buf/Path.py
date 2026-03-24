from pathlib import Path
from typing import override

from editor.core import REPLCommand, ProgramState
from editor.core.context import EditorContext, PathUtils


class BufPathCommand(REPLCommand):
	def __init__(self, program: ProgramState):
		super().__init__(program, "buf.path")

	@override
	def execute(self):
		EditorContext.assert_initialized(self.program.project_dir)

		if len(self.program.args) == 0:
			self.print_arg_error("Expected at least 1 argument")
		else:
			for arg in self.program.args:
				self.path(arg)

	@override
	def help(self):
		print("help not implemented")  # DOC

	def path(self, arg: str):
		asset = Path(arg).resolve()
		if not asset.exists():
			self.print_arg_error(f"{PathUtils.printed_path(asset)} does not exist")
			return

		buffer_path = EditorContext.data_buffer_path(self.program.project_dir, asset)
		print(PathUtils.printed_path(buffer_path), f"({'open' if buffer_path.exists() else 'closed'})")

def register(program: ProgramState):
	program.machine.default().add_command(BufPathCommand(program))
