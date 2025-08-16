import os.path
from pathlib import Path

from PySide6.QtWidgets import QWidget, QFileDialog

from editor import ui
from editor.util import ProjectContext


class ContentBrowser(QWidget):
	def __init__(self):
		super().__init__()
		self.win = None
		self.ui = ui.ContentBrowser.Ui_ContentBrowser()
		self.ui.setupUi(self)

		self.res_folder = ProjectContext.project_resource_folder()

		self.ui.newAsset.clicked.connect(self.new_asset)
		self.ui.browseFolder.clicked.connect(self.browse_folder)

		self.current_folder = self.res_folder
		self.last_file_dialog_dir = self.current_folder
		self.open_folder(self.current_folder)

	def open_folder(self, folder):
		if not Path(folder).is_relative_to(self.res_folder):
			return

		self.current_folder = folder
		rel_folder = Path(folder).relative_to(self.res_folder)
		self.ui.folderLineEdit.setText("RES://" + rel_folder.name)
		# TODO v3 populate icons

	def browse_folder(self, folder):
		folder = QFileDialog.getExistingDirectory(self, "Select Folder", self.last_file_dialog_dir)
		if folder:
			self.last_file_dialog_dir = folder
			self.open_folder(folder)

	def new_asset(self):
		pass  # TODO
