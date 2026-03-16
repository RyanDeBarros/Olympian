from typing import override, Iterable

from prompt_toolkit.completion import CompleteEvent, Completion
from prompt_toolkit.document import Document

from editor.core.REPL import REPLCommand, ProgramState
from editor.tools import eprint


class HelpCommand(REPLCommand):
	def __init__(self, program: ProgramState):
		super().__init__(program, "help")

	@override
	def execute(self):
		if len(self.program.args) == 0:
			self.help()
		elif len(self.program.args) == 1:
			# TODO v7 custom autocomplete for argument
			# TODO v7 support any command, not just command in current state
			if self.program.args[0] in self.program.machine.state().commands:
				self.program.machine.state().commands[self.program.args[0]].help()
			else:
				eprint("Argument is not a valid command")
		else:
			eprint("Expected 0-1 arguments")

	@override
	def help(self):
		pass  # DOC

	def get_completions(self, document: Document, complete_event: CompleteEvent) -> Iterable[Completion]:
		# TODO v7 support any commands, not just in current state
		yield from self.program.machine.state().get_command_completions(document)


def register(program: ProgramState):
	program.machine.default.add_command(HelpCommand(program))
