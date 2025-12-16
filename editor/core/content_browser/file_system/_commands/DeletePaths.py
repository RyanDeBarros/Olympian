from pathlib import Path
from typing import Optional

from PySide6.QtGui import QUndoCommand

from editor.core.path_items import get_path_item
from .._Notifier import _InternalOps, _Notifier


class DeletePaths(QUndoCommand):
	def __init__(self, notifier: _Notifier, paths: list[Path]):
		super().__init__("Delete Path(s)")
		self.notifier = notifier
		self.num_paths = len(paths)

		self.paths = paths
		self.trash_paths: Optional[list[Path]] = None
		self.hash_path: Optional[Path] = None

	def _generate_trash_paths(self):
		self.hash_path = self.notifier.ops.generate_hash_path()
		self.trash_paths = [self.hash_path / path.relative_to(self.notifier.project_context.project_folder) for
							path in self.paths]

	def undo(self):
		for i in range(self.num_paths):
			_InternalOps.move_to(self.trash_paths[i], self.paths[i])
			import_path = Path(self.trash_paths[i].as_posix() + '.oly')
			if import_path.exists():
				_InternalOps.move_to(import_path, Path(self.paths[i].as_posix() + '.oly'))
		_InternalOps.rm_all_dirs(self.hash_path)
		self.trash_paths = None
		self.hash_path = None
		self.notifier.notify_browser_add_paths(self.paths)

		for path in self.paths:
			item = get_path_item(path)
			assert item is not None
			item.on_new(self.notifier.content_browser)

	def redo(self):
		self._generate_trash_paths()
		self.notifier.notify_browser_remove_paths(self.paths)
		self.notifier.notify_main_tab_remove_paths(self.paths)

		for path in self.paths:
			item = get_path_item(path)
			assert item is not None
			item.on_delete(self.notifier.content_browser)

		for i in range(self.num_paths):
			_InternalOps.move_to(self.paths[i], self.trash_paths[i])
			import_path = Path(self.paths[i].as_posix() + '.oly')
			if import_path.exists():
				_InternalOps.move_to(import_path, Path(self.trash_paths[i].as_posix() + '.oly'))
