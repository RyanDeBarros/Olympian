import os
from pathlib import Path
from typing import override

from editor.core.REPL import REPLCommand, ProgramState
from editor.tools import eprint


class ProjectOpenCommand(REPLCommand):
	def __init__(self, program: ProgramState):
		super().__init__(program, "project.open")

	@override
	def execute(self):
		if len(self.program.args) == 1:
			cwd = Path(self.program.args[0])
			if cwd.is_dir():
				os.chdir(cwd)
				self.program.project_dir = Path(cwd).resolve()
			else:
				eprint(f"{cwd.as_posix()} is not a valid directory")
		else:
			eprint(f"Expected 1 argument")

	@override
	def help(self):
		print("help not implemented")  # DOC


def register(program: ProgramState):
	program.machine.default().add_command(ProjectOpenCommand(program))
