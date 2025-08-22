import posixpath
from pathlib import Path

from PySide6.QtGui import QUndoStack


class ProjectContext:
	def __init__(self, project_file):
		self.project_file = project_file
		self.project_folder = Path(posixpath.dirname(self.project_file))
		self.res_folder = self.project_folder.joinpath("res")
		self.undo_stack = QUndoStack()  # TODO v3 save undo stack to file so it can be loaded

		from editor.util.FileIOMachine import FileIOMachine
		self.file_machine = FileIOMachine(self)

