from enum import Enum
from typing import Iterable

from prompt_toolkit.completion import Completion
from prompt_toolkit.document import Document

from . import REPLState, REPLCommand


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
