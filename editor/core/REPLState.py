from typing import Iterable

from prompt_toolkit.completion import CompleteEvent, Completion
from prompt_toolkit.document import Document

from . import REPLCommand, PathCompleter


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
			yield from self.get_command_completions(document, self.command_strings)
		else:
			cmd = words[0]
			if cmd in self.commands:
				yield from self.commands[cmd].get_completions(document, complete_event)
			else:
				yield from PathCompleter.get_path_completions(document)

	@staticmethod
	def get_command_completions(document: Document, command_strings: list[str]) -> Iterable[Completion]:
		cword = document.get_word_before_cursor(WORD=True)

		typed_parts = cword.split(".")
		current_part = typed_parts[-1]
		complete_parts = typed_parts[:-1]

		seen: set[str] = set()

		for cmd in command_strings:
			parts = cmd.split(".")

			if len(typed_parts) > len(parts):
				continue

			if parts[:len(complete_parts)] != complete_parts:
				continue

			next_part = parts[len(complete_parts)]
			if not next_part.startswith(current_part):
				continue

			suggestion = next_part
			if len(parts) > len(complete_parts) + 1:
				suggestion += "."

			if suggestion not in seen:
				seen.add(suggestion)
				yield Completion(suggestion, start_position=-len(current_part))
