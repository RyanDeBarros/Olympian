from typing import Iterable

from prompt_toolkit.completion import CompleteEvent, Completion
from prompt_toolkit.document import Document

from . import REPLCommand, PathCompleter, KeyCompleter


class REPLState:
	def __init__(self):
		self.commands: dict[str, REPLCommand] = {}
		self.command_strings: list[str] = []

	def cache_commands(self):
		self.command_strings = sorted(self.commands.keys())

	def add_command(self, command: REPLCommand):
		assert command.name not in self.commands
		self.commands[command.name] = command

	def get_completions(self, document: Document, complete_event: CompleteEvent) -> Iterable[Completion]:
		words = document.text_before_cursor.split()
		if len(words) <= 1 and not document.text_before_cursor.endswith(" "):
			yield from KeyCompleter.get_keys_completions(document, self.command_strings)
		else:
			cmd = words[0]
			if cmd in self.commands:
				yield from self.commands[cmd].get_completions(document, complete_event)
			else:
				yield from PathCompleter.get_path_completions(document)
