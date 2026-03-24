import os
import shutil
import subprocess
from typing import override

from editor.core import REPLCommand, ProgramState


class ShellExecCommand(REPLCommand):
	def __init__(self):
		super().__init__("shell.exec")

	@override
	def requires_initialized_editor(self):
		return False

	@override
	def execute(self):
		program = ProgramState.instance()
		if len(program.args) == 0:
			self.print_arg_error("Expected at least 1 argument")
			return

		bash = shutil.which("bash")

		try:
			if os.name == "nt" and bash:
				subprocess.run([bash, "-c", str(' '.join(program.args))])
			else:
				subprocess.run(' '.join(program.args), shell=True)

		except Exception as e:
			self.print_arg_error(f"Shell error: {e}")

	@override
	def help(self):
		print("help not implemented")  # DOC


def register():
	ProgramState.instance().machine.default().add_command(ShellExecCommand())
