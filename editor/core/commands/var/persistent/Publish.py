from typing import override

from editor.core.REPL import REPLCommand, ProgramState
from .. import Storage


# TODO v7 here and in other var commands, add autocomplete for variable key names that exist
# TODO v7 here and in other var commands, accept '*'-prefixed args for: all vars replace in persistent (override existing), or replace all vars if they don't already exist in persistent.
class VarPersistentPublishCommand(REPLCommand):
	def __init__(self, program: ProgramState):
		super().__init__(program, "var.persistent.publish")

	@override
	def execute(self):
		if len(self.program.args) == 0:
			self.print_arg_error("Expected at least 1 argument")
		else:
			for arg in self.program.args:
				try:
					value = Storage.get_temp(arg)
				except KeyError:
					self.print_arg_error(f"Key does not exist: {arg}")
				else:
					Storage.set_persistent(arg, value)

	@override
	def help(self):
		print("help not implemented")  # DOC


def register(program: ProgramState):
	program.machine.default().add_command(VarPersistentPublishCommand(program))


# TODO v7 buffers:
#    * data: for asset I/O
#    * editor settings
#    * macro (temp) var I/O
#    * large output dump: add options to certain commands like list to print in that buffer instead of cmdline
#    * error: instead of in cmdline, print asset syntax/format errors in buffer. Just print in cmdline that the error buffer should be checked
#    Multiple data/error buffers so multiple assets can be open at the same time?
#    buffers.<cmd> <buffer-name>: custom autocomplete for buffer names in argument
#    buffers.open: open with default app (likely IDE/VSCode). To make this work consistently, use custom file extension for buffers (.olybuf?)
#    buffers.reveal: open buffer in file explorer. The path should be in AppData, under some subfolder that corresponds to a manifest mapping of the project, to allow for multiple buffers for different projects.
#    buffers.path: print path of buffer file
