import os
import sys
from abc import ABC, abstractmethod
from pathlib import Path
from typing import Iterable, override

from prompt_toolkit import PromptSession
from prompt_toolkit.completion import Completer, CompleteEvent, Completion
from prompt_toolkit.document import Document
from prompt_toolkit.key_binding import KeyBindings, KeyPressEvent
from prompt_toolkit.shortcuts import CompleteStyle


class ProgramState:
	def __init__(self, project_dir: Path):
		self.exit = False
		self.project_dir = project_dir.resolve()
		self.argline = ""
		self.args = []

	def load_args(self, argline: str):
		self.argline = argline
		self.args = argline.split()  # TODO v7 handle quoted args

	def project_name(self) -> str:
		return self.project_dir.name

	def cwd_prompt(self) -> str:
		cwd = Path(os.getcwd()).relative_to(self.project_dir).as_posix()
		return f"oly {self.project_name()}/{cwd if cwd != '.' else ''} > "


# TODO v7 handle quoting paths with spaces
def get_path_completions(document: Document) -> Iterable[Completion]:
	cword = document.get_word_before_cursor(WORD=True)

	dir_part = os.path.dirname(cword) if os.path.dirname(cword) else '.'
	prefix = os.path.basename(cword)

	try:
		for special in ['.', '..']:
			if special.startswith(prefix):
				yield Completion(special + '/', start_position=-len(prefix))

		for f in os.listdir(dir_part):
			if f.startswith(prefix):
				full_path = os.path.join(dir_part, f)
				if os.path.isdir(full_path):
					f += '/'
				yield Completion(f, start_position=-len(prefix))
	except FileNotFoundError:
		pass  # skip


class REPLCommand(ABC):
	def __init__(self, name: str):
		self.name = name

	@abstractmethod
	def execute(self, program: ProgramState):
		raise NotImplementedError()

	def get_completions(self, document: Document, complete_event: CompleteEvent) -> Iterable[Completion]:
		yield from get_path_completions(document)


class REPLState:
	def __init__(self):
		self.commands: dict[str, REPLCommand] = {}

	def add_command(self, command: REPLCommand):
		self.commands[command.name] = command

	def command_strings(self):
		return sorted(self.commands.keys())

	def get_completions(self, document: Document, complete_event: CompleteEvent) -> Iterable[Completion]:
		words = document.text_before_cursor.split()
		if len(words) <= 1 and not document.text_before_cursor.endswith(" "):
			yield from self.get_command_completions(document)
		else:
			cmd = words[0]
			if cmd in self.commands:
				yield from self.commands[cmd].get_completions(document, complete_event)
			else:
				yield from get_path_completions(document)

	def get_command_completions(self, document: Document) -> Iterable[Completion]:
		cword = document.get_word_before_cursor(WORD=True)

		typed_parts = cword.split(".")
		current_part = typed_parts[-1]
		complete_parts = typed_parts[:-1]

		seen: set[str] = set()

		for cmd in self.command_strings():
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
def _(event: KeyPressEvent):
	buf = event.app.current_buffer
	buf.cancel_completion()


@kb.add("enter")
def _(event: KeyPressEvent):
	buf = event.app.current_buffer
	if buf.complete_state and buf.complete_state.current_completion:
		buf.apply_completion(buf.complete_state.current_completion)
		buf.complete_state = None
	else:
		event.app.current_buffer.validate_and_handle()


def run() -> None:
	machine = REPLStateMachine()

	from editor.core import commands
	commands.register(machine)

	completer = REPLCompleter(machine)
	session = PromptSession(completer=completer, complete_while_typing=False, complete_style=CompleteStyle.COLUMN, key_bindings=kb)

	program = ProgramState(Path(os.getcwd()).resolve())

	while True:
		command: str = session.prompt(program.cwd_prompt())

		elements = command.split()
		if len(elements) == 0:
			continue

		cmd = elements[0]

		if cmd in machine.state().commands:
			program.load_args(command[len(cmd):].strip())
			machine.state().commands[cmd].execute(program)

			if program.exit:
				break
		else:
			print(f"Unrecognized command: {cmd}", file=sys.stderr)
