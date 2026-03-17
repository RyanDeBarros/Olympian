import os
from pathlib import Path
from typing import Iterable, override

from prompt_toolkit import PromptSession
from prompt_toolkit.completion import Completer, CompleteEvent, Completion
from prompt_toolkit.document import Document
from prompt_toolkit.key_binding import KeyBindings, KeyPressEvent
from prompt_toolkit.shortcuts import CompleteStyle

from editor.core import REPLError, REPLStateMachine, ProgramState
from editor.tools import eprint


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
			except REPLError as e:
				e.print()
				continue

			cmd.execute()

			if program.exit:
				break
		else:
			eprint(f"Unrecognized command: {cmd_name}")
