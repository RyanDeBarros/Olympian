import os
import string
from pathlib import Path
import random
from typing import Optional

import send2trash
from PySide6.QtGui import QUndoCommand

from editor.core import ProjectContext


class UCRenamePath(QUndoCommand):
	def __init__(self, old_path: Path, new_path: Path):
		super().__init__("Rename Path")
		self.old_path = old_path
		self.new_path = new_path

	def undo(self):
		assert not os.path.exists(self.old_path)
		os.makedirs(os.path.dirname(self.old_path), exist_ok=True)
		os.rename(self.new_path, self.old_path)

	def redo(self):
		assert not os.path.exists(self.new_path)
		os.makedirs(os.path.dirname(self.new_path), exist_ok=True)
		os.rename(self.old_path, self.new_path)


class FileIOMachine:
	def __init__(self, project_context: ProjectContext):
		self.project_context = project_context
		self.undo_stack = self.project_context.undo_stack

	@staticmethod
	def _send_to_trash(path):
		send2trash.send2trash(Path(path).resolve())

	def refresh_folder_view(self):
		return self.project_context.main_window.content_browser.folder_view.refresh_view()

	def _trash_folder(self) -> Path:  # TODO v3 add .trash to project-specific .gitignore
		return self.project_context.project_folder.joinpath(".trash")

	def remove(self, path):
		self.undo_stack.push(UCDeletePaths(self, [Path(path).resolve()]))

	def remove_together(self, paths):
		self.undo_stack.push(UCDeletePaths(self, [Path(path).resolve() for path in paths]))

	def clear_trash(self):
		self._send_to_trash(self._trash_folder())


class UCDeletePaths(QUndoCommand):
	def __init__(self, machine: FileIOMachine, paths: list[Path]):
		super().__init__("Delete Path(s)")
		self.machine = machine
		self.num_paths = len(paths)

		self.paths = paths
		self.trash_paths: Optional[list[Path]] = None
		self.hash_path: Optional[Path] = None

	def _generate_trash_paths(self):
		relative_paths = [path.relative_to(self.machine.project_context.project_folder) for path in self.paths]
		self.hash_path = self.machine._trash_folder().joinpath(self._generate_hash_container())
		self.trash_paths = [self.hash_path.joinpath(relative_path) for relative_path in relative_paths]

	def _generate_hash_container(self):
		h = ''.join(random.choices(string.ascii_lowercase + string.digits, k=8))
		if os.path.exists(self.machine._trash_folder().joinpath(h)):
			return self._generate_hash_container()
		else:
			return h

	def undo(self):
		for i in range(self.num_paths):
			path = self.paths[i]
			trash_path = self.trash_paths[i]
			assert not os.path.exists(path)
			os.makedirs(os.path.dirname(path), exist_ok=True)
			os.rename(trash_path, path)

		for root, dirs, _ in os.walk(self.hash_path, topdown=False):
			for d in dirs:
				os.rmdir(os.path.join(root, d))
		os.rmdir(self.hash_path)
		self.machine.refresh_folder_view()  # TODO v3 instead of refreshing the entire view, just add the paths that were added to the currently open folder
		self.trash_paths = None
		self.hash_path = None

	def redo(self):
		self._generate_trash_paths()
		for i in range(self.num_paths):
			path = self.paths[i]
			trash_path = self.trash_paths[i]
			assert not os.path.exists(trash_path)
			os.makedirs(os.path.dirname(trash_path), exist_ok=True)
			os.rename(path, trash_path)

		self.machine.refresh_folder_view()  # TODO v3 instead of refreshing the entire view, just remove the paths that were removed from the currently open folder
