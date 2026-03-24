import os
import platform
import subprocess
from pathlib import Path

from editor.core import REPLError, ProgramState
from editor.tools import eprint


class EditorContext:
	CONTEXT_ROOT_NAME: str = '.editor'
	BUFFER_FILE_EXTENSION: str = 'olybuf'

	@staticmethod
	def context_root(project_dir: Path) -> Path:
		return project_dir / EditorContext.CONTEXT_ROOT_NAME

	@staticmethod
	def is_initialized(project_dir: Path) -> bool:
		return EditorContext.context_root(project_dir).exists() and EditorContext.context_root(project_dir).is_dir()

	@staticmethod
	def assert_initialized(project_dir: Path) -> None:
		if not EditorContext.is_initialized(project_dir):
			raise REPLError("Editor is not initialized")

	@staticmethod
	def initialize(program: ProgramState) -> None:
		if EditorContext.is_initialized(program.project_dir):
			return

		context_root = EditorContext.context_root(program.project_dir)

		if context_root.exists():
			eprint(".editor exists, but is not initialized")
			return

		if any(path == EditorContext.CONTEXT_ROOT_NAME for path in context_root.parents):
			eprint("Cannot initialize editor - an editor already exists in a parent directory")
			return

		context_root.mkdir(exist_ok=False)
		program.macros.persistent_path().touch(exist_ok=False)
		program.settings.persistent_path().touch(exist_ok=False)

	@staticmethod
	def data_root(project_dir: Path) -> Path:
		return EditorContext.context_root(project_dir) / 'data'

	@staticmethod
	def data_buffer_path(project_dir: Path, asset_path: Path) -> Path:
		rel_path = asset_path if not asset_path.is_absolute() else asset_path.relative_to(project_dir)
		return EditorContext.data_root(project_dir) / f"{rel_path}.{EditorContext.BUFFER_FILE_EXTENSION}"

	@staticmethod
	def reveal_in_explorer(path: Path) -> None:
		if not path.exists():
			raise REPLError(f"Path {str(path)} does not exist")

		if platform.system() == "Windows":
			subprocess.run(["explorer", "/select,", str(path)])
		elif platform.system() == "Darwin":
			subprocess.run(["open", "-R", str(path)])
		else:
			try:
				subprocess.run(["nautilus", "--select", str(path)])
			except FileNotFoundError:
				try:
					subprocess.run(["dolphin", "--select", str(path)])
				except FileNotFoundError:
					subprocess.run(["xdg-open", str(path.parent)])

	@staticmethod
	def open_with_default_app(path: Path) -> None:
		if not path.exists():
			raise REPLError(f"Path {str(path)} does not exist")

		if platform.system() == "Windows":
			os.startfile(str(path))
		elif platform.system() == "Darwin":
			subprocess.run(["open", str(path)])
		else:
			subprocess.run(["xdg-open", str(path)])
