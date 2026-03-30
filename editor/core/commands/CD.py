import os
from pathlib import Path
from typing import override, Iterable

from prompt_toolkit.completion import CompleteEvent, Completion
from prompt_toolkit.document import Document

from editor.core import REPLCommand, ProgramState, PathCompleter
from editor.core.context import PathUtils


class CDCommand(REPLCommand):
	def __init__(self):
		super().__init__("cd")

	@override
	def requires_initialized_editor(self):
		return False

	@override
	def execute(self):
		program = ProgramState.instance()
		if len(program.args) == 1:
			cwd = Path(program.args[0])
			if cwd.is_dir():
				if cwd.absolute().resolve().is_relative_to(program.project_dir):
					os.chdir(cwd)
			else:
				self.print_arg_error(f"{PathUtils.printed_path(cwd)} is not a valid directory")
		else:
			self.print_arg_error(f"Expected 1 argument")

	@override
	def help(self):
		print("help not implemented")  # DOC

	@override
	def get_completions(self, document: Document, complete_event: CompleteEvent) -> Iterable[Completion]:
		yield from PathCompleter.get_path_completions(document, directories_only=True)


def register():
	ProgramState.instance().machine.default().add_command(CDCommand())
