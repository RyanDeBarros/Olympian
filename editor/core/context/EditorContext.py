import os
import platform
import subprocess
from pathlib import Path
from typing import Iterator, TYPE_CHECKING

from editor.core import REPLError
from editor.core.ProgramState import ProgramState
from editor.tools import eprint

if TYPE_CHECKING:
	from editor.core.buffers import BufferPath


class EditorContext:
	CONTEXT_ROOT_NAME: str = '.editor'
	BUFFER_FILE_EXTENSION: str = 'olybuf'

	@staticmethod
	def context_root(project_dir: Path | None = None) -> Path:
		if project_dir is None:
			project_dir = ProgramState.instance().project_dir
		return project_dir / EditorContext.CONTEXT_ROOT_NAME

	@staticmethod
	def is_initialized() -> bool:
		return EditorContext.context_root().is_dir()

	@staticmethod
	def assert_initialized() -> None:
		if not EditorContext.is_initialized():
			raise REPLError("Editor is not initialized")

	@staticmethod
	def initialize() -> None:
		if EditorContext.is_initialized():
			return

		context_root = EditorContext.context_root()

		if context_root.exists():
			eprint(".editor exists, but is not initialized")
			return

		if any(path == EditorContext.CONTEXT_ROOT_NAME for path in context_root.parents):
			eprint("Cannot initialize editor - an editor already exists in a parent directory")
			return

		context_root.mkdir(exist_ok=False)
		program = ProgramState.instance()
		program.macros.persistent_path.touch(exist_ok=False)
		program.settings.persistent_path.touch(exist_ok=False)

	@staticmethod
	def macros_path(project_dir: Path | None = None) -> Path:
		return EditorContext.context_root(project_dir) / f'macros.{EditorContext.BUFFER_FILE_EXTENSION}'

	@staticmethod
	def settings_path(project_dir: Path | None = None) -> Path:
		return EditorContext.context_root(project_dir) / f'settings.{EditorContext.BUFFER_FILE_EXTENSION}'

	@staticmethod
	def data_root() -> Path:
		return EditorContext.context_root() / 'data'

	@staticmethod
	def data_buffers() -> Iterator["BufferPath"]:
		from editor.core.buffers import BufferPath
		for path in EditorContext.data_root().rglob(f"*.{EditorContext.BUFFER_FILE_EXTENSION}"):
			yield BufferPath.from_buffer(path)

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
			subprocess.run(['explorer.exe', str(path)], creationflags=0x00000008 | 0x08000000)
		elif platform.system() == "Darwin":
			subprocess.run(["open", str(path)])
		else:
			subprocess.run(["xdg-open", str(path)])
