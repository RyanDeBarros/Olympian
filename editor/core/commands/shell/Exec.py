import os
import shutil
import subprocess
from typing import override

from editor.core import REPLCommand, ProgramState


class ShellExecCommand(REPLCommand):
	def __init__(self, program: ProgramState):
		super().__init__(program, "shell.exec")

	@override
	def execute(self):
		if len(self.program.args) == 0:
			self.print_arg_error("Expected at least 1 argument")
			return

		bash = shutil.which("bash")

		try:
			if os.name == "nt" and bash:
				subprocess.run([bash, "-c", ' '.join(self.program.args)])
			else:
				subprocess.run(' '.join(self.program.args), shell=True)

		except Exception as e:
			self.print_arg_error(f"Shell error: {e}")

	@override
	def help(self):
		print("help not implemented")  # DOC


def register(program: ProgramState):
	program.machine.default().add_command(ShellExecCommand(program))
