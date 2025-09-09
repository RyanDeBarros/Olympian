from idlelib.outwin import file_line_pats
from pathlib import Path

from PySide6.QtWidgets import QWidget, QFileDialog


class FileDialog:
	def __init__(self, at_dir: Path):
		self.last_dir = at_dir
		self.backup_dir = self.last_dir

	# noinspection PyShadowingBuiltins
	def get_open_file_name(self, parent: QWidget | None = ..., caption: str = ..., filter: str = ...) -> Path | None:
		filepath, _ = QFileDialog.getOpenFileName(parent=parent, caption=caption, dir=self.last_dir.as_posix(),
												  filter=filter)
		if filepath:
			filepath = Path(filepath).resolve()
			self.backup_dir = self.last_dir
			self.last_dir = filepath.parent
			return filepath
		else:
			return None

	def get_existing_directory(self, parent: QWidget | None = ..., caption: str = ...) -> Path | None:
		folder = QFileDialog.getExistingDirectory(parent=parent, caption=caption, dir=self.last_dir.as_posix())
		if folder:
			folder = Path(folder).resolve()
			self.backup_dir = self.last_dir
			self.last_dir = folder
			return folder
		else:
			return None

	def reset_last_dir(self):
		self.last_dir = self.backup_dir
