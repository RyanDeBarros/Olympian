from typing import override, Iterable

from prompt_toolkit.completion import CompleteEvent, Completion
from prompt_toolkit.document import Document

from editor.core import REPLCommand, ProgramState, KeyCompleter
from editor.core.context import EditorContext


class VarPersistentPublishCommand(REPLCommand):
	def __init__(self):
		super().__init__("var.persistent.publish")
		self.star_override = '*o'
		self.star_checked = '*c'

	@override
	def execute(self):
		EditorContext.assert_initialized()

		program = ProgramState.instance()
		if len(program.args) == 0:
			self.print_arg_error("Expected at least 1 argument")
		elif program.args == [self.star_override]:
			for key in program.macros.temporary.keys():
				value = program.macros.temporary.get(key)
				program.macros.persistent.set(key, value)
		elif program.args == [self.star_checked]:
			for key in program.macros.temporary.keys():
				if key not in program.macros.persistent.keys():
					value = program.macros.temporary.get(key)
					program.macros.persistent.set(key, value)
		else:
			for arg in program.args:
				try:
					value = program.macros.temporary.get(arg)
				except KeyError:
					self.print_arg_error(f"Key does not exist: {arg}")
				else:
					program.macros.persistent.set(arg, value)

	@override
	def help(self):
		print("help not implemented")  # DOC

	@override
	def get_completions(self, document: Document, complete_event: CompleteEvent) -> Iterable[Completion]:
		yield from KeyCompleter.get_keys_completions(document, ProgramState.instance().macros.temporary.keys() + [self.star_override, self.star_checked])


def register():
	ProgramState.instance().machine.default().add_command(VarPersistentPublishCommand())
