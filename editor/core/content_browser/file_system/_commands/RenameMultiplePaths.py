from pathlib import Path

from PySide6.QtGui import QUndoCommand

from .RenamePath import RenamePath
from .._Notifier import _Notifier


class RenameMultiplePaths(QUndoCommand):
	def __init__(self, notifier: _Notifier, old_paths: list[Path], new_paths: list[Path],
				 replace_existing: list[bool]):
		super().__init__("Rename Multiple Paths")
		self.notifier = notifier
		assert len(old_paths) == len(new_paths) and len(old_paths) == len(replace_existing)
		self.num_paths = len(old_paths)
		self.renamers = [RenamePath(self.notifier, old_path, new_path, replace) for old_path, new_path, replace in
						 zip(old_paths, new_paths, replace_existing)]

	def undo(self):
		for i in range(self.num_paths):
			self.renamers[i].undo()

	def redo(self):
		for i in range(self.num_paths):
			self.renamers[i].redo()
