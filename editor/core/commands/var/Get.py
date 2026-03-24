from typing import override, Iterable

from prompt_toolkit.completion import CompleteEvent, Completion
from prompt_toolkit.document import Document

from editor.core import Resolver, REPLError, REPLCommand, ProgramState, KeyCompleter


class VarGetCommand(REPLCommand):
	def __init__(self):
		super().__init__("var.get")

	@override
	def requires_initialized_editor(self):
		return False

	@override
	def execute(self):
		program = ProgramState.instance()
		if len(program.args) == 0:
			self.print_arg_error("Expected at least 1 argument")
		else:
			for arg in program.args:
				try:
					value = program.macros.temporary.get(arg)
				except KeyError:
					self.print_arg_error(f"${arg} is not an existing temp var")
				else:
					print(f"${arg} = {value}")
					try:
						expanded = Resolver.expand_macros(program, arg)
						if expanded != value:
							print(f"(expanded) ${arg} = {expanded}")
					except REPLError as e:
						print(f"Failed to expand ${arg}: {e.description}")

	@override
	def help(self):
		print("help not implemented")  # DOC

	@override
	def get_completions(self, document: Document, complete_event: CompleteEvent) -> Iterable[Completion]:
		yield from KeyCompleter.get_keys_completions(document, ProgramState.instance().macros.temporary.keys())


def register():
	ProgramState.instance().machine.default().add_command(VarGetCommand())
