from typing import override, Iterable

from prompt_toolkit.completion import CompleteEvent, Completion
from prompt_toolkit.document import Document

from editor.core import REPLCommand, ProgramState, KeyCompleter
from .. import Storage


class VarPersistentPublishCommand(REPLCommand):
	def __init__(self, program: ProgramState):
		super().__init__(program, "var.persistent.publish")
		self.star_override = '*o'
		self.star_checked = '*c'

	@override
	def execute(self):
		if len(self.program.args) == 0:
			self.print_arg_error("Expected at least 1 argument")
		elif self.program.args == [self.star_override]:
			for key in Storage.temp_keys():
				value = Storage.get_temp(key)
				Storage.set_persistent(key, value)
		elif self.program.args == [self.star_checked]:
			for key in Storage.temp_keys():
				if key not in Storage.persistent_keys():
					value = Storage.get_temp(key)
					Storage.set_persistent(key, value)
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

	@override
	def get_completions(self, document: Document, complete_event: CompleteEvent) -> Iterable[Completion]:
		yield from KeyCompleter.get_keys_completions(document, Storage.temp_keys() + [self.star_override, self.star_checked])


def register(program: ProgramState):
	program.machine.default().add_command(VarPersistentPublishCommand(program))
