import os
from abc import ABC, abstractmethod
from enum import Enum
from pathlib import Path
from typing import Iterable, override

from prompt_toolkit import PromptSession
from prompt_toolkit.completion import Completer, CompleteEvent, Completion
from prompt_toolkit.document import Document
from prompt_toolkit.key_binding import KeyBindings, KeyPressEvent
from prompt_toolkit.shortcuts import CompleteStyle

from editor.core import Resolver
from editor.tools import eprint


# DOC document that paths are wrapped with [], and document macro usage $^
def get_path_completions(document: Document) -> Iterable[Completion]:
	text_before_cursor = document.text_before_cursor

	opening_index = text_before_cursor.rfind(Resolver.GROUP_OPEN)
	if opening_index == -1:
		return

	cword = text_before_cursor[opening_index + 1:]

	dir_part = os.path.dirname(cword) if os.path.dirname(cword) else '.'
	prefix = os.path.basename(cword)

	try:
		for special in ['.', '..']:
			if special.startswith(prefix):
				yield Completion(f'{special}/', start_position=-len(prefix))

		for f in os.listdir(dir_part):
			if f.startswith(prefix):
				full_path = os.path.join(dir_part, f)
				if os.path.isdir(full_path):
					completion = f'{f}/'
				else:
					completion = f'{f}{Resolver.GROUP_CLOSE}'
				yield Completion(completion, start_position=-len(prefix))
	except FileNotFoundError:
		pass


class REPLCommand(ABC):
	def __init__(self, program: "ProgramState", name: str):
		self.program = program
		self.name = name

	@abstractmethod
	def execute(self):
		raise NotImplementedError()

	@abstractmethod
	def help(self):
		raise NotImplementedError()

	def expand_macros(self):
		return True

	def get_completions(self, document: Document, complete_event: CompleteEvent) -> Iterable[Completion]:
		yield from get_path_completions(document)

	def print_arg_error(self, msg: str):
		eprint(f"{msg}. Run `help {self.name}` for more details")


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
				yield from get_path_completions(document)

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


class REPLStateMachine:
	class State(Enum):
		DEFAULT = 0

	def __init__(self):
		self.states = {
			REPLStateMachine.State.DEFAULT: REPLState()
		}
		self.all_commands: dict[str, REPLCommand] = {}
		self.all_command_strings: list[str] = []

	def state(self) -> REPLState:
		return self.default()

	def default(self):
		return self.states[REPLStateMachine.State.DEFAULT]

	def cache_commands(self):
		for state in self.states.values():
			state.cache_commands()
			self.all_commands.update(state.commands)
		self.all_command_strings = sorted(self.all_commands.keys())

	def get_all_command_completions(self, document: Document) -> Iterable[Completion]:
		yield from REPLState.get_command_completions(document, self.all_command_strings)


class REPLCompleter(Completer):
	def __init__(self, machine: REPLStateMachine):
		self.machine = machine

	@override
	def get_completions(self, document: Document, complete_event: CompleteEvent) -> Iterable[Completion]:
		yield from self.machine.state().get_completions(document, complete_event)


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


@kb.add("c-h")
def _(event: KeyPressEvent):
	buf = event.app.current_buffer
	length = 1

	if event.key_sequence[0].data == '\x08':  # ctrl+backspace
		cword = buf.document.get_word_before_cursor(WORD=True)
		if cword:
			length = len(cword)
		else:
			text = buf.document.text_before_cursor
			length = len(text) - len(text.rstrip())

	buf.delete_before_cursor(count=length)


class ProgramState:
	def __init__(self, project_dir: Path):
		self.machine = REPLStateMachine()
		self.exit = False
		self.project_dir = project_dir.resolve()
		self.argline = ""
		self.expanded_argline = ""
		self.args: list[str] = []

	def load_args(self, argline: str, expand_macros: bool):
		self.argline = argline
		if expand_macros:
			self.expanded_argline = Resolver.expand_macros(self.argline)
		else:
			self.expanded_argline = self.argline
		self.args = Resolver.split_groups(self.expanded_argline)

	def project_name(self) -> str:
		return self.project_dir.name

	def cwd_prompt(self) -> str:
		cwd = Path(os.getcwd()).relative_to(self.project_dir).as_posix()
		return f"oly [{self.project_name()}/{cwd if cwd != '.' else ''}] > "


def run() -> None:
	program = ProgramState(Path(os.getcwd()).resolve())

	# Load commands
	from . import commands
	commands.register(program)
	program.machine.cache_commands()

	# Setup prompter
	completer = REPLCompleter(program.machine)
	session = PromptSession(completer=completer, complete_while_typing=False, complete_style=CompleteStyle.COLUMN, key_bindings=kb)

	while True:
		try:
			command: str = session.prompt(program.cwd_prompt())
		except (KeyboardInterrupt, EOFError):
			break

		elements = command.split()
		if len(elements) == 0:
			continue

		cmd_name = elements[0]

		if cmd_name in program.machine.state().commands:
			cmd = program.machine.state().commands[cmd_name]

			try:
				program.load_args(command[len(cmd_name):].strip(), expand_macros=cmd.expand_macros())
			except ValueError:
				eprint("Invalid syntax")  # TODO v7 more descriptive error depending on [/]/macro invalidity
				continue

			cmd.execute()

			if program.exit:
				break
		else:
			eprint(f"Unrecognized command: {cmd_name}")
