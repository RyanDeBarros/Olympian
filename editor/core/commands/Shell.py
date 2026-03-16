import os
import shutil
import subprocess
from typing import override

from editor.core.REPL import REPLCommand, ProgramState
from editor.tools import eprint


class ShellCommand(REPLCommand):
	def __init__(self, program: ProgramState):
		super().__init__(program, "shell")

	@override
	def execute(self):
		if not self.program.argline:
			eprint("Expected at least 1 argument")
			return

		bash = shutil.which("bash")

		try:
			if os.name == "nt" and bash:
				subprocess.run([bash, "-c", self.program.argline])
			else:
				subprocess.run(self.program.argline, shell=True)

		except Exception as e:
			eprint(f"Shell error: {e}")

	@override
	def help(self):
		print("help not implemented")  # DOC


def register(program: ProgramState):
	program.machine.default.add_command(ShellCommand(program))
