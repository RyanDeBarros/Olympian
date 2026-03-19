from typing import override, Iterable

from prompt_toolkit.completion import CompleteEvent, Completion
from prompt_toolkit.document import Document

from editor.core import REPLCommand, ProgramState, KeyCompleter
from .. import Storage


class VarPersistentLoadCommand(REPLCommand):
	def __init__(self, program: ProgramState):
		super().__init__(program, "var.persistent.load")
		self.star_override = '*o'
		self.star_checked = '*c'

	@override
	def execute(self):
		if len(self.program.args) == 0:
			for key in Storage.persistent_keys():
				self.load(key)
		elif self.program.args == [self.star_override]:
			for key in Storage.persistent_keys():
				value = Storage.get_persistent(key)
				Storage.set_temp(key, value)
		elif self.program.args == [self.star_checked]:
			for key in Storage.persistent_keys():
				if key not in Storage.temp_keys():
					value = Storage.get_persistent(key)
					Storage.set_temp(key, value)
		else:
			for arg in self.program.args:
				self.load(arg)

	def load(self, key: str):
		try:
			value = Storage.get_persistent(key)
		except KeyError:
			self.print_arg_error(f"${key} is not a persistent var")
		else:
			Storage.set_temp(key, value)

	@override
	def help(self):
		print("help not implemented")  # DOC

	@override
	def get_completions(self, document: Document, complete_event: CompleteEvent) -> Iterable[Completion]:
		yield from KeyCompleter.get_keys_completions(document, Storage.persistent_keys() + [self.star_override, self.star_checked])


def register(program: ProgramState):
	program.machine.default().add_command(VarPersistentLoadCommand(program))
