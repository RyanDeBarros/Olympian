import os
from pathlib import Path

from . import _commands
from ._InternalOps import _InternalOps
from ._Notifier import _Notifier


class FileSystemMachine:
	from editor.core.ProjectContext import ProjectContext
	def __init__(self, project_context: ProjectContext):
		self.project_context = project_context
		self._ops = _InternalOps(project_context)
		self._notifier = _Notifier(project_context, self._ops)
		self.undo_stack = self._ops.content_browser.undo_stack

	def clear_trash(self):
		self._ops.send_to_trash(self.project_context.trash_folder)
		os.makedirs(self.project_context.trash_folder)

	def remove(self, path: Path):
		self.undo_stack.push(_commands.DeletePaths(self._notifier, self._ops.resolve_all([path])))

	def remove_together(self, paths: list[Path]):
		self.undo_stack.push(_commands.DeletePaths(self._notifier, self._ops.resolve_all(paths)))

	def rename(self, old_path: Path, new_path: Path, replace_existing: bool):
		self.undo_stack.push(_commands.RenamePath(self._notifier, Path(old_path).resolve(), Path(new_path).resolve(), replace_existing))

	def rename_all(self, old_paths: list[Path], new_paths: list[Path], replace_existing: list[bool]):
		length = self._ops.assert_same_length(old_paths, new_paths, replace_existing)
		if length == 1:
			self.rename(old_paths[0], new_paths[0], replace_existing[0])
		elif length > 1:
			self.undo_stack.push(_commands.RenameMultiplePaths(self._notifier, self._ops.resolve_all(old_paths), self._ops.resolve_all(new_paths), replace_existing))

	def copy_paste(self, copied_paths: list[Path], pasted_paths: list[Path], replace_existing: list[bool]):
		length = self._ops.assert_same_length(copied_paths, pasted_paths, replace_existing)
		if length > 0:
			self.undo_stack.push(_commands.CopyPastePaths(self._notifier, self._ops.resolve_all(copied_paths), self._ops.resolve_all(pasted_paths), replace_existing))

	def new_folder(self, folder: Path):
		self.undo_stack.push(_commands.NewFolder(self._notifier, Path(folder).resolve()))

	def new_file(self, file: Path):
		self.undo_stack.push(_commands.NewFile(self._notifier, Path(file).resolve()))
