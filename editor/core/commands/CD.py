import os
from pathlib import Path
from typing import override

from editor.core.REPL import REPLCommand, ProgramState
from editor.tools import eprint


class CDCommand(REPLCommand):
	def __init__(self, program: ProgramState):
		super().__init__(program, "cd")

	@override
	def execute(self):
		if len(self.program.args) == 1:
			cwd = Path(self.program.args[0])
			if cwd.is_dir():
				if cwd.absolute().resolve().is_relative_to(self.program.project_dir):
					os.chdir(cwd)
			else:
				eprint(f"{cwd.as_posix()} is not a valid directory")
		else:
			eprint(f"Expected 1 argument")

	@override
	def help(self):
		print("help not implemented")  # DOC


def register(program: ProgramState):
	program.machine.default.add_command(CDCommand(program))
