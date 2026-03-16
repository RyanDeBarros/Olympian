import os
import shutil
import subprocess
from typing import override

from editor.core.REPL import REPLCommand, REPLStateMachine, ProgramState
from editor.tools import eprint


class ShellCommand(REPLCommand):
	def __init__(self):
		super().__init__("shell")

	@override
	def execute(self, program: ProgramState):
		if not program.argline:
			eprint("Expected at least 1 argument")
			return

		bash = shutil.which("bash")

		try:
			if os.name == "nt" and bash:
				subprocess.run([bash, "-c", program.argline])
			else:
				subprocess.run(program.argline, shell=True)

		except Exception as e:
			eprint(f"Shell error: {e}")


def register(machine: REPLStateMachine):
	machine.default.add_command(ShellCommand())
