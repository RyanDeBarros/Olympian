import os
import string
from pathlib import Path
import random
from typing import Optional

import send2trash
from PySide6.QtGui import QUndoCommand

from editor.core import ProjectContext


class FileIOMachine:
	def __init__(self, project_context: ProjectContext):
		self.project_context = project_context
		self.undo_stack = self.project_context.undo_stack

	@staticmethod
	def _send_to_trash(path):
		send2trash.send2trash(Path(path).resolve())

	def clear_trash(self):
		self._send_to_trash(self._trash_folder())
		os.makedirs(self._trash_folder())

	def _generate_hash_container(self):
		h = ''.join(random.choices(string.ascii_lowercase + string.digits, k=8))
		if os.path.exists(self._trash_folder().joinpath(h)):
			return self._generate_hash_container()
		else:
			return h

	def generate_hash_path(self):
		return self._trash_folder().joinpath(self._generate_hash_container())

	def refresh_folder_view(self):
		return self.project_context.main_window.content_browser.folder_view.refresh_view()

	def _trash_folder(self) -> Path:
		return self.project_context.project_folder.joinpath(".trash")

	def remove(self, path: Path):
		self.undo_stack.push(UCDeletePaths(self, [Path(path).resolve()]))

	def remove_together(self, paths: list[Path]):
		self.undo_stack.push(UCDeletePaths(self, [Path(path).resolve() for path in paths]))

	def rename(self, old_path: Path, new_path: Path):
		self.undo_stack.push(UCRenamePath(self, Path(old_path).resolve(), Path(new_path).resolve()))

	def new_folder(self, folder: Path):
		self.undo_stack.push(UCNewFolder(self, Path(folder).resolve()))

	def new_file(self, file: Path):
		self.undo_stack.push(UCNewFile(self, Path(file).resolve()))


class UCDeletePaths(QUndoCommand):
	def __init__(self, machine: FileIOMachine, paths: list[Path]):
		super().__init__("Delete Path(s)")
		self.machine = machine
		self.num_paths = len(paths)

		self.paths = paths
		self.trash_paths: Optional[list[Path]] = None
		self.hash_path: Optional[Path] = None

	def _generate_trash_paths(self):
		self.hash_path = self.machine.generate_hash_path()
		self.trash_paths = [self.hash_path.joinpath(path.relative_to(self.machine.project_context.project_folder)) for path in self.paths]

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


class UCRenamePath(QUndoCommand):
	def __init__(self, machine: FileIOMachine, old_path: Path, new_path: Path):
		super().__init__("Rename Path")
		self.machine = machine
		self.old_path = old_path
		self.new_path = new_path
		self.do_refresh = False

	def undo(self):
		assert not os.path.exists(self.old_path)
		os.makedirs(os.path.dirname(self.old_path), exist_ok=True)
		os.rename(self.new_path, self.old_path)
		self.machine.refresh_folder_view()  # TODO v3 instead of refreshing the entire view, just rename the item if in currently open folder

	def redo(self):
		assert not os.path.exists(self.new_path)
		os.makedirs(os.path.dirname(self.new_path), exist_ok=True)
		os.rename(self.old_path, self.new_path)
		if self.do_refresh:
			self.machine.refresh_folder_view()  # TODO v3 instead of refreshing the entire view, just rename the item if in currently open folder
		else:
			self.do_refresh = True


class UCNewFolder(QUndoCommand):
	def __init__(self, machine: FileIOMachine, folder: Path):
		super().__init__("New Folder")
		self.machine = machine
		self.folder = folder
		self.makedirs = True
		self.hash_path: Optional[Path] = None
		self.trash_path: Optional[Path] = None

	def _generate_trash_path(self):
		self.hash_path = self.machine.generate_hash_path()
		self.trash_path = self.hash_path.joinpath(self.folder.relative_to(self.machine.project_context.project_folder))

	def undo(self):
		self._generate_trash_path()
		assert not os.path.exists(self.trash_path)
		os.makedirs(os.path.dirname(self.trash_path), exist_ok=True)
		os.rename(self.folder, self.trash_path)
		self.machine.refresh_folder_view()  # TODO v3 instead of refreshing the entire view, just remove the item if in currently open folder

	def redo(self):
		if self.makedirs:
			self.makedirs = False
			os.makedirs(self.folder)
		else:
			assert not os.path.exists(self.folder)
			os.makedirs(os.path.dirname(self.folder), exist_ok=True)
			os.rename(self.trash_path, self.folder)

			for root, dirs, _ in os.walk(self.hash_path, topdown=False):
				for d in dirs:
					os.rmdir(os.path.join(root, d))
			os.rmdir(self.hash_path)
			self.trash_path = None
			self.hash_path = None
			self.machine.refresh_folder_view()  # TODO v3 instead of refreshing the entire view, just add the item if in currently open folder


class UCNewFile(QUndoCommand):
	def __init__(self, machine: FileIOMachine, file: Path):
		super().__init__("New File")
		self.machine = machine
		self.file = file
		self.touch = True
		self.hash_path: Optional[Path] = None
		self.trash_path: Optional[Path] = None

	def _generate_trash_path(self):
		self.hash_path = self.machine.generate_hash_path()
		self.trash_path = self.hash_path.joinpath(self.file.relative_to(self.machine.project_context.project_folder))

	def undo(self):
		self._generate_trash_path()
		assert not os.path.exists(self.trash_path)
		os.makedirs(os.path.dirname(self.trash_path), exist_ok=True)
		os.rename(self.file, self.trash_path)
		self.machine.refresh_folder_view()  # TODO v3 instead of refreshing the entire view, just remove the item if in currently open folder

	def redo(self):
		if self.touch:
			self.touch = False
			self.file.touch(exist_ok=False)
		else:
			assert not os.path.exists(self.file)
			os.makedirs(os.path.dirname(self.file), exist_ok=True)
			os.rename(self.trash_path, self.file)

			for root, dirs, _ in os.walk(self.hash_path, topdown=False):
				for d in dirs:
					os.rmdir(os.path.join(root, d))
			os.rmdir(self.hash_path)
			self.trash_path = None
			self.hash_path = None
			self.machine.refresh_folder_view()  # TODO v3 instead of refreshing the entire view, just add the item if in currently open folder
