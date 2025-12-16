import os
from pathlib import Path
from typing import Optional

from PySide6.QtGui import QUndoCommand

from editor.core.path_items import get_path_item
from .._Notifier import _InternalOps, _Notifier


class NewFolder(QUndoCommand):
	def __init__(self, notifier: _Notifier, folder: Path):
		super().__init__("New Folder")
		self.notifier = notifier
		self.folder = folder
		self.makedirs = True
		self.hash_path: Optional[Path] = None
		self.trash_path: Optional[Path] = None

	def _generate_trash_path(self):
		self.hash_path = self.notifier.ops.generate_hash_path()
		self.trash_path = self.hash_path / self.folder.relative_to(self.notifier.project_context.project_folder)

	def undo(self):
		item = get_path_item(self.folder)
		assert item is not None
		item.on_delete(self.notifier.content_browser)

		_InternalOps.move_to(self.folder, self.trash_path)

		self._generate_trash_path()
		self.notifier.notify_browser_remove_path(self.folder)

	def redo(self):
		if self.makedirs:
			self.makedirs = False
			os.makedirs(self.folder)
		else:
			_InternalOps.move_to(self.trash_path, self.folder)
			_InternalOps.rm_all_dirs(self.hash_path)
			self.trash_path = None
			self.hash_path = None
			self.notifier.notify_browser_add_path(self.folder)

		item = get_path_item(self.folder)
		assert item is not None
		item.on_new(self.notifier.content_browser)
