import os
import sys
from abc import ABC, abstractmethod
from typing import Iterable, override

from prompt_toolkit import PromptSession
from prompt_toolkit.completion import Completer, CompleteEvent, Completion
from prompt_toolkit.document import Document
from prompt_toolkit.key_binding import KeyBindings
from prompt_toolkit.shortcuts import CompleteStyle


class ProgramState:
	def __init__(self):
		self.exit = False


class REPLCommand(ABC):
	def __init__(self, name: str):
		self.name = name

	@abstractmethod
	def execute(self, program: ProgramState, args: list[str]):
		pass


class REPLState:
	def __init__(self):
		self.commands: dict[str, REPLCommand] = {}

	def add_command(self, command: REPLCommand):
		self.commands[command.name] = command

	def command_strings(self):
		return sorted(self.commands.keys())

	def get_completions(self, document: Document, complete_event: CompleteEvent) -> Iterable[Completion]:
		words = document.text_before_cursor.split()

		# TODO v7 use current state to determine commands list instead of this if-else branching
		if len(words) <= 1 and not document.text_before_cursor.endswith(" "):
			cword = document.get_word_before_cursor(WORD=False)
			commands = self.command_strings()
			for cmd in commands:
				if cmd.startswith(cword):
					yield Completion(cmd, start_position=-len(cword))
		else:
			# TODO v7 use REPLCommand context for completion
			cword = document.get_word_before_cursor(WORD=True)
			for f in os.listdir("."):
				if f.startswith(cword):
					yield Completion(f, start_position=-len(cword))


class REPLStateMachine:
	def __init__(self):
		self.default = REPLState()

	def state(self):
		return self.default


class REPLCompleter(Completer):
	def __init__(self, machine: REPLStateMachine):
		self.machine = machine

	@override
	def get_completions(self, document: Document, complete_event: CompleteEvent) -> Iterable[Completion]:
		yield from self.machine.default.get_completions(document, complete_event)  # TODO v7 possible option later on to switch states


kb = KeyBindings()


@kb.add("c-w")
def _(event):
	event.app.current_buffer.cancel_completion()


def run() -> None:
	machine = REPLStateMachine()

	from editor.core import commands
	commands.register_commands(machine)

	completer = REPLCompleter(machine)
	session = PromptSession(completer=completer, complete_while_typing=False, complete_style=CompleteStyle.COLUMN, key_bindings=kb)

	program = ProgramState()

	while True:
		rel_dir = "\"Project Name\""  # TODO v7 use cwd relative to project folder. project folder is either the starting cwd or a cmdline arg passed to OlyEditor
		command = session.prompt(f"oly {rel_dir} > ")

		elements = command.split()
		if len(elements) == 0:
			continue

		cmd = elements[0]

		if cmd in machine.state().commands:
			machine.state().commands[cmd].execute(program, elements[1:])

			if program.exit:
				break
		else:
			print(f"Unrecognized command: {cmd}", file=sys.stderr)
