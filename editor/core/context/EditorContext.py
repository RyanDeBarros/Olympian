from pathlib import Path

from editor.core import ProgramState
from editor.tools import eprint


class EditorContext:
	@staticmethod
	def context_root_name():
		return '.editor'

	@staticmethod
	def context_root(program: ProgramState) -> Path:
		return program.project_dir / EditorContext.context_root_name()

	@staticmethod
	def is_initialized(program: ProgramState) -> bool:
		return EditorContext.context_root(program).exists() and EditorContext.context_root(program).is_dir()

	@staticmethod
	def assert_initialized(program: ProgramState) -> None:
		assert EditorContext.is_initialized(program)

	@staticmethod
	def initialize(program: ProgramState) -> None:
		if EditorContext.is_initialized(program):
			return

		context_root = EditorContext.context_root(program)

		if context_root.exists():
			eprint(".editor exists, but is not initialized")
			return

		if any(path == EditorContext.context_root_name() for path in context_root.parents):
			eprint("Cannot initialize editor - an editor already exists in a parent directory")
			return

		context_root.mkdir(exist_ok=False)

	@staticmethod
	def data_root(program: ProgramState) -> Path:
		return EditorContext.context_root(program) / 'data'

	@staticmethod
	def data_buffer_path(program: ProgramState, asset_path: Path) -> Path:
		rel_path = asset_path if not asset_path.is_absolute() else asset_path.relative_to(program.project_dir)
		return EditorContext.data_root(program) / rel_path
