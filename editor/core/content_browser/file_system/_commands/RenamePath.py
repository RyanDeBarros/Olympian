from pathlib import Path

from PySide6.QtGui import QUndoCommand

from editor.core.path_items import get_path_item
from .DeletePaths import DeletePaths
from .._Notifier import _InternalOps, _Notifier


class RenamePath(QUndoCommand):
	def __init__(self, notifier: _Notifier, old_path: Path, new_path: Path, replace_existing: bool):
		super().__init__("Rename Path")
		self.notifier = notifier
		self.old_path = old_path
		self.new_path = new_path
		self.replacer = DeletePaths(self.notifier, [self.new_path]) if replace_existing else None

	def undo(self):
		self.notifier.notify_browser_remove_path(self.new_path)
		self.notifier.notify_main_tab_rename_path(self.new_path, self.old_path)
		_InternalOps.move_to(self.new_path, self.old_path)
		import_path = Path(self.new_path.as_posix() + '.oly')
		if import_path.exists():
			_InternalOps.move_to(import_path, Path(self.old_path.as_posix() + '.oly'))
		self.notifier.notify_browser_add_path(self.old_path)

		item = get_path_item(self.old_path)
		assert item is not None
		item.on_rename(self.notifier.content_browser, self.new_path)

		if self.replacer is not None:
			self.replacer.undo()

	def redo(self):
		if self.replacer is not None:
			self.replacer.redo()

		self.notifier.notify_browser_remove_path(self.old_path)
		self.notifier.notify_main_tab_rename_path(self.old_path, self.new_path)
		_InternalOps.move_to(self.old_path, self.new_path)
		import_path = Path(self.old_path.as_posix() + '.oly')
		if import_path.exists():
			_InternalOps.move_to(import_path, Path(self.new_path.as_posix() + '.oly'))
		self.notifier.notify_browser_add_path(self.new_path)

		item = get_path_item(self.new_path)
		assert item is not None
		item.on_rename(self.notifier.content_browser, self.old_path)
