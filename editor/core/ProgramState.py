import os
from pathlib import Path

from . import Resolver
from .context import PathUtils


class ProgramState:
	_instance = None

	def __init__(self, project_dir: Path):
		# project
		self.project_dir = project_dir.resolve()

		# repl
		from . import REPLStateMachine
		self.machine = REPLStateMachine()
		self.exit = False
		self.argline = ""
		self.expanded_argline = ""
		self.args: list[str] = []

		# storage
		from editor.core.context import EditorContext
		from .commands.var.MacroStorage import MacroStorage
		self.macros = MacroStorage(EditorContext.macros_path(self.project_dir))
		from .commands.editor.settings import EditorSettings
		self.settings = EditorSettings(EditorContext.settings_path(self.project_dir))

	def late_init(self):
		if self.macros.persistent_path.is_file():
			self.macros.load_persistent()

		if self.settings.persistent_path.is_file():
			self.settings.load()

	@classmethod
	def instance(cls, project_dir=None):
		if cls._instance is None:
			if project_dir is None:
				raise ValueError("First call must provide project_dir")
			cls._instance = cls(project_dir)
		return cls._instance

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
		cwd = Path(os.getcwd()).relative_to(self.project_dir)
		path = self.project_name() / (cwd if cwd != '.' else '')
		return f"oly [{PathUtils.printed_path(path)}] > "
