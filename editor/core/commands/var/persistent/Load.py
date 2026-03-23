from typing import override, Iterable

from prompt_toolkit.completion import CompleteEvent, Completion
from prompt_toolkit.document import Document

from editor.core import REPLCommand, ProgramState, KeyCompleter
from editor.core.context import EditorContext


class VarPersistentLoadCommand(REPLCommand):
	def __init__(self, program: ProgramState):
		super().__init__(program, "var.persistent.load")
		self.star_override = '*o'
		self.star_checked = '*c'

	@override
	def execute(self):
		EditorContext.assert_initialized(self.program.project_dir)
		
		if len(self.program.args) == 0:
			for key in self.program.macros.persistent.keys():
				self.load(key)
		elif self.program.args == [self.star_override]:
			for key in self.program.macros.persistent.keys():
				value = self.program.macros.persistent.get(key)
				self.program.macros.temporary.set(key, value)
		elif self.program.args == [self.star_checked]:
			for key in self.program.macros.persistent.keys():
				if key not in self.program.macros.temporary.keys():
					value = self.program.macros.persistent.get(key)
					self.program.macros.temporary.set(key, value)
		else:
			for arg in self.program.args:
				self.load(arg)

	def load(self, key: str):
		try:
			value = self.program.macros.persistent.get(key)
		except KeyError:
			self.print_arg_error(f"${key} is not a persistent var")
		else:
			self.program.macros.temporary.set(key, value)

	@override
	def help(self):
		print("help not implemented")  # DOC

	@override
	def get_completions(self, document: Document, complete_event: CompleteEvent) -> Iterable[Completion]:
		yield from KeyCompleter.get_keys_completions(document, self.program.macros.persistent.keys() + [self.star_override, self.star_checked])


def register(program: ProgramState):
	program.machine.default().add_command(VarPersistentLoadCommand(program))
