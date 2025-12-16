from pathlib import Path
from typing import Optional

from PySide6.QtGui import QUndoCommand

from editor.core.path_items import get_path_item
from .._Notifier import _InternalOps, _Notifier


class NewFile(QUndoCommand):
	def __init__(self, notifier: _Notifier, file: Path):
		super().__init__("New File")
		self.notifier = notifier
		self.file = file
		self.touch = True
		self.hash_path: Optional[Path] = None
		self.trash_path: Optional[Path] = None

	def _generate_trash_path(self):
		self.hash_path = self.notifier.ops.generate_hash_path()
		self.trash_path = self.hash_path / self.file.relative_to(self.notifier.project_context.project_folder)

	def undo(self):
		self._generate_trash_path()
		self.notifier.notify_browser_remove_path(self.file)
		self.notifier.notify_main_tab_remove_path(self.file)

		item = get_path_item(self.file)
		assert item is not None
		item.on_delete(self.notifier.content_browser)

		_InternalOps.move_to(self.file, self.trash_path)
		import_path = Path(self.file.as_posix() + '.oly')
		if import_path.exists():
			_InternalOps.move_to(import_path, Path(self.trash_path.as_posix() + '.oly'))

	def redo(self):
		if self.touch:
			self.touch = False
			self.file.touch()
		else:
			_InternalOps.move_to(self.trash_path, self.file)
			import_path = Path(self.trash_path.as_posix() + '.oly')
			if import_path.exists():
				_InternalOps.move_to(import_path, Path(self.file.as_posix() + '.oly'))
			_InternalOps.rm_all_dirs(self.hash_path)
			self.trash_path = None
			self.hash_path = None
			self.notifier.notify_browser_add_path(self.file)

		item = get_path_item(self.file)
		assert item is not None
		item.on_new(self.notifier.content_browser)
