import os
import sys
from pathlib import Path
from typing import override

from editor.core.REPL import REPLCommand, REPLStateMachine, ProgramState


class ProjectOpenCommand(REPLCommand):
	def __init__(self):
		super().__init__("project.open")

	@override
	def execute(self, program: ProgramState):
		if len(program.args) == 1:
			cwd = Path(program.args[0])
			if cwd.is_dir():
				os.chdir(cwd)
				program.project_dir = Path(cwd).resolve()
			else:
				print(f"{cwd.as_posix()} is not a valid directory", file=sys.stderr)
		else:
			print(f"Expected 1 argument", file=sys.stderr)


def register(machine: REPLStateMachine):
	machine.default.add_command(ProjectOpenCommand())
