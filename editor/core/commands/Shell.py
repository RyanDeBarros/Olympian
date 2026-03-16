import os
import shutil
import subprocess
import sys
from typing import override

from editor.core.REPL import REPLCommand, REPLStateMachine, ProgramState


class ShellCommand(REPLCommand):
	def __init__(self):
		super().__init__("shell")

	@override
	def execute(self, program: ProgramState):
		if not program.argline:
			print("Expected at least 1 argument", file=sys.stderr)
			return

		bash = shutil.which("bash")

		try:
			if os.name == "nt" and bash:
				subprocess.run([bash, "-c", program.argline])
			else:
				subprocess.run(program.argline, shell=True)

		except Exception as e:
			print(f"Shell error: {e}", file=sys.stderr)


def register(machine: REPLStateMachine):
	machine.default.add_command(ShellCommand())
