from typing import override, Iterable

from prompt_toolkit.completion import CompleteEvent, Completion
from prompt_toolkit.document import Document

from editor.core import REPLCommand, ProgramState, KeyCompleter


class VarPersistentLoadCommand(REPLCommand):
	def __init__(self):
		super().__init__("var.persistent.load")
		self.star_override = '*o'
		self.star_checked = '*c'

	@override
	def execute(self):
		program = ProgramState.instance()
		if len(program.args) == 0:
			for key in program.macros.persistent.keys():
				self.load(key)
		elif program.args == [self.star_override]:
			for key in program.macros.persistent.keys():
				value = program.macros.persistent.get(key)
				program.macros.temporary.set(key, value)
		elif program.args == [self.star_checked]:
			for key in program.macros.persistent.keys():
				if key not in program.macros.temporary.keys():
					value = program.macros.persistent.get(key)
					program.macros.temporary.set(key, value)
		else:
			for arg in program.args:
				self.load(arg)

	def load(self, key: str):
		program = ProgramState.instance()
		try:
			value = program.macros.persistent.get(key)
		except KeyError:
			self.print_arg_error(f"${key} is not a persistent var")
		else:
			program.macros.temporary.set(key, value)

	@override
	def help(self):
		print("help not implemented")  # DOC

	@override
	def get_completions(self, document: Document, complete_event: CompleteEvent) -> Iterable[Completion]:
		yield from KeyCompleter.get_keys_completions(document, ProgramState.instance().macros.persistent.keys() + [self.star_override, self.star_checked])


def register():
	ProgramState.instance().machine.default().add_command(VarPersistentLoadCommand())
