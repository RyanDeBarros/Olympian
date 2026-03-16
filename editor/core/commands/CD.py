import os
from pathlib import Path
from typing import override

from core.REPL import REPLCommand, REPLStateMachine, ProgramState
from tools import eprint


class CDCommand(REPLCommand):
	def __init__(self):
		super().__init__("cd")

	@override
	def execute(self, program: ProgramState):
		if len(program.args) == 1:
			cwd = Path(program.args[0])
			if cwd.is_dir():
				if cwd.absolute().resolve().is_relative_to(program.project_dir):
					os.chdir(cwd)
			else:
				eprint(f"{cwd.as_posix()} is not a valid directory")
		else:
			eprint(f"Expected 1 argument")


def register(machine: REPLStateMachine):
	machine.default.add_command(CDCommand())
