import shutil
from pathlib import Path

from PySide6.QtGui import QUndoCommand

from editor.core.path_items import get_path_item
from .DeletePaths import DeletePaths
from .._Notifier import _Notifier


class CopyPastePaths(QUndoCommand):
	def __init__(self, notifier: _Notifier, copied_paths: list[Path], pasted_paths: list[Path],
				 replace_existing: list[bool]):
		super().__init__("Paste Copied Paths")
		self.notifier = notifier
		assert len(copied_paths) == len(pasted_paths) and len(copied_paths) == len(replace_existing)
		self.copied_paths = copied_paths
		self.pasted_paths = pasted_paths
		self.num_paths = len(self.copied_paths)
		self.replacers = [DeletePaths(self.notifier, [pasted_path]) if replace else None for replace, pasted_path in
						 zip(replace_existing, self.pasted_paths)]
		self.first_paste = True
		self.paste_deleter = DeletePaths(self.notifier, self.pasted_paths)

	def undo(self):
		self.paste_deleter.redo()
		for i in range(self.num_paths):
			if self.replacers[i] is not None:
				self.replacers[i].undo()

	def redo(self):
		for i in range(self.num_paths):
			if self.replacers[i] is not None:
				self.replacers[i].redo()

		if self.first_paste:
			for i in range(self.num_paths):
				if self.copied_paths[i].is_file():
					shutil.copy2(self.copied_paths[i], self.pasted_paths[i])
					import_file = Path(self.copied_paths[i].as_posix() + '.oly')
					if import_file.exists():
						shutil.copy2(import_file, Path(self.pasted_paths[i].as_posix() + '.oly'))
				elif self.copied_paths[i].is_dir():
					shutil.copytree(self.copied_paths[i], self.pasted_paths[i])
				else:
					assert False
		else:
			self.paste_deleter.undo()

		if self.first_paste:
			self.first_paste = False
			for i in range(self.num_paths):
				self.notifier.notify_browser_add_path(self.pasted_paths[i])

				item = get_path_item(self.pasted_paths[i])
				assert item is not None
				item.on_new(self.notifier.content_browser)
