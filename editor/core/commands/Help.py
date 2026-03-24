from typing import override, Iterable

from prompt_toolkit.completion import CompleteEvent, Completion
from prompt_toolkit.document import Document

from editor.core import REPLCommand, ProgramState


class HelpCommand(REPLCommand):
	def __init__(self):
		super().__init__("help")

	@override
	def execute(self):
		program = ProgramState.instance()
		if len(program.args) == 0:
			self.help()
		elif len(program.args) == 1:
			if program.args[0] in program.machine.all_commands:
				program.machine.all_commands[program.args[0]].help()
			else:
				self.print_arg_error("Argument is not a valid command")
		else:
			self.print_arg_error("Expected 0-1 arguments")

	@override
	def help(self):
		print('help not implemented')  # DOC

	def get_completions(self, document: Document, complete_event: CompleteEvent) -> Iterable[Completion]:
		yield from ProgramState.instance().machine.get_all_command_completions(document)


def register():
	ProgramState.instance().machine.default().add_command(HelpCommand())
