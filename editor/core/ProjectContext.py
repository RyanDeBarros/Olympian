import posixpath
from pathlib import Path


class ProjectContext:
	from editor.core import MainWindow

	def __init__(self, project_file, main_window: MainWindow):
		self.project_file = project_file
		self.project_folder = Path(posixpath.dirname(self.project_file))
		self.res_folder = self.project_folder.joinpath("res")
		self.main_window = main_window
