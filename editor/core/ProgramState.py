import os
from pathlib import Path

from editor.core import Resolver


class ProgramState:
	def __init__(self, project_dir: Path):
		from . import REPLStateMachine
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
