import posixpath
from pathlib import Path

from PySide6.QtGui import QUndoStack


class ProjectContext:
	from editor.core import MainWindow

	def __init__(self, project_file, main_window: MainWindow):
		self.project_file = project_file
		self.project_folder = Path(posixpath.dirname(self.project_file))
		self.res_folder = self.project_folder.joinpath("res")
		self.main_window = main_window
		self.undo_stack = QUndoStack()  # TODO v3 save undo stack to file so it can be loaded
		self.undo_stack.setUndoLimit(0)  # TODO v3 editor preference for undo limit

		from editor.util.FileIOMachine import FileIOMachine
		self.file_machine = FileIOMachine(self)

