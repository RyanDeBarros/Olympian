import os
from typing import Iterable, override

from prompt_toolkit import PromptSession
from prompt_toolkit.completion import Completer, CompleteEvent, Completion
from prompt_toolkit.document import Document
from prompt_toolkit.key_binding import KeyBindings
from prompt_toolkit.shortcuts import CompleteStyle


class REPLCompleter(Completer):
	@override
	def get_completions(self, document: Document, complete_event: CompleteEvent) -> Iterable[Completion]:
		words = document.text_before_cursor.split()

		# TODO v7 use current state to determine commands list instead of this if-else branching
		if len(words) <= 1 and not document.text_before_cursor.endswith(" "):
			cword = document.get_word_before_cursor(WORD=False)
			commands = ["exit", "help"]
			for cmd in commands:
				if cmd.startswith(cword):
					yield Completion(cmd, start_position=-len(cword))
		else:
			cword = document.get_word_before_cursor(WORD=True)
			for f in os.listdir("."):
				if f.startswith(cword):
					yield Completion(f, start_position=-len(cword))


kb = KeyBindings()


@kb.add("c-w")
def _(event):
	event.app.current_buffer.cancel_completion()


completer = REPLCompleter()
session = PromptSession(completer=completer, complete_while_typing=False, complete_style=CompleteStyle.COLUMN, key_bindings=kb)


def run() -> None:
	while True:
		rel_dir = "\"Project Name\""  # TODO v7 use cwd relative to project folder. project folder is either the starting cwd or a cmdline arg passed to OlyEditor
		command = session.prompt(f"oly {rel_dir} > ")

		elements = command.split()
		if elements[0] == "exit":
			break

		print("=", command)
