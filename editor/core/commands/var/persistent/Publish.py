from typing import override, Iterable

from prompt_toolkit.completion import CompleteEvent, Completion
from prompt_toolkit.document import Document

from editor.core import REPLCommand, ProgramState, KeyCompleter
from editor.core.context import EditorContext


class VarPersistentPublishCommand(REPLCommand):
	def __init__(self, program: ProgramState):
		super().__init__(program, "var.persistent.publish")
		self.star_override = '*o'
		self.star_checked = '*c'

	@override
	def execute(self):
		EditorContext.assert_initialized(self.program.project_dir)

		if len(self.program.args) == 0:
			self.print_arg_error("Expected at least 1 argument")
		elif self.program.args == [self.star_override]:
			for key in self.program.macros.temporary.keys():
				value = self.program.macros.temporary.get(key)
				self.program.macros.persistent.set(key, value)
		elif self.program.args == [self.star_checked]:
			for key in self.program.macros.temporary.keys():
				if key not in self.program.macros.persistent.keys():
					value = self.program.macros.temporary.get(key)
					self.program.macros.persistent.set(key, value)
		else:
			for arg in self.program.args:
				try:
					value = self.program.macros.temporary.get(arg)
				except KeyError:
					self.print_arg_error(f"Key does not exist: {arg}")
				else:
					self.program.macros.persistent.set(arg, value)

	@override
	def help(self):
		print("help not implemented")  # DOC

	@override
	def get_completions(self, document: Document, complete_event: CompleteEvent) -> Iterable[Completion]:
		yield from KeyCompleter.get_keys_completions(document, self.program.macros.temporary.keys() + [self.star_override, self.star_checked])


def register(program: ProgramState):
	program.machine.default().add_command(VarPersistentPublishCommand(program))
