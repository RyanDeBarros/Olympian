import os
from pathlib import Path
from typing import override

from editor.core import REPLCommand, ProgramState
from editor.core.context import PathUtils


class CDCommand(REPLCommand):
	def __init__(self):
		super().__init__("cd")

	@override
	def requires_initialized_editor(self):
		return False

	@override
	def execute(self):
		program = ProgramState.instance()
		if len(program.args) == 1:
			cwd = Path(program.args[0])
			if cwd.is_dir():
				if cwd.absolute().resolve().is_relative_to(program.project_dir):
					os.chdir(cwd)
			else:
				self.print_arg_error(f"{PathUtils.printed_path(cwd)} is not a valid directory")
		else:
			self.print_arg_error(f"Expected 1 argument")

	@override
	def help(self):
		print("help not implemented")  # DOC


def register():
	ProgramState.instance().machine.default().add_command(CDCommand())
