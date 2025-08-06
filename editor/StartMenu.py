import posixpath
import re

from PySide6.QtWidgets import QWidget, QFileDialog

import ui
from editor import ManifestTOML


class StartMenuWidget(QWidget):
	def __init__(self, manifest: ManifestTOML):
		super().__init__()
		self.manifest = manifest
		self.ui = ui.startmenu.Ui_Form()
		self.ui.setupUi(self)

		self.new_tab = NewTab(self)
		self.open_tab = OpenTab(self)
		self.recent_tab = RecentTab(self)

	def open_project(self, project_filepath):
		# TODO
		print(f"Opening {project_filepath}...")
		self.close()


class NewTab:
	def __init__(self, start_menu: StartMenuWidget):
		self.start_menu = start_menu
		self.project_name = start_menu.ui.projectName
		self.project_folder = start_menu.ui.folderName
		self.browse = start_menu.ui.newBrowseButton
		self.create_project_folder = start_menu.ui.createProjectFolderButton
		self.src_folder = start_menu.ui.srcFolder
		self.res_folder = start_menu.ui.resFolder
		self.gen_folder = start_menu.ui.genFolder
		self.project_filepath = start_menu.ui.projectFilepath
		self.create_project_button = start_menu.ui.createProjectButton
		self.error_message = start_menu.ui.errorCreateProject

		self.project_name.textChanged.connect(self.sync_project_name)
		self.project_folder.textChanged.connect(self.sync_project_name)
		self.browse.clicked.connect(self.open_browse)
		self.create_project_folder.checkStateChanged.connect(self.sync_project_name)
		self.create_project_button.clicked.connect(self.create_project)

		self.sync_project_name()

	@staticmethod
	def is_valid_project_name(name):
		if len(name) == 0:
			return False
		return bool(re.fullmatch(r'[A-Za-z0-9_\- ]+', name))

	def generated_path(self, path):
		project = self.project_name.text()
		folder = self.project_folder.text()
		if NewTab.is_valid_project_name(project) and len(folder) > 0:
			if self.create_project_folder.isChecked():
				folder = posixpath.join(folder, project)
			return posixpath.join(folder, path)
		else:
			return ""

	def generated_src_folder(self):
		return self.generated_path("src/")

	def generated_res_folder(self):
		return self.generated_path("res/")

	def generated_gen_folder(self):
		return self.generated_path(".gen/")

	def generated_project_filepath(self):
		return self.generated_path(f"{self.project_name.text()}.oly")

	def sync_project_name(self):
		self.src_folder.setText(self.generated_src_folder())
		self.res_folder.setText(self.generated_res_folder())
		self.gen_folder.setText(self.generated_gen_folder())
		project_filepath = self.generated_project_filepath()
		self.project_filepath.setText(project_filepath)

		disable = project_filepath == ""
		if disable:
			self.error_message.setText("")
		else:
			disable = self.project_is_contained_in_existing_project()
			if disable:
				self.error_message.setText("A project already exists in a parent folder!")
			else:
				self.error_message.setText("")
		self.create_project_button.setDisabled(disable)

	def project_is_contained_in_existing_project(self):
		project = self.project_name.text()
		folder = self.project_folder.text()
		if NewTab.is_valid_project_name(project) and len(folder) > 0:
			if self.create_project_folder.isChecked():
				folder = posixpath.join(folder, project)
			return self.start_menu.manifest.filepath_is_contained_in_existing_project(folder)
		else:
			return False

	def open_browse(self):
		folder_path = QFileDialog.getExistingDirectory(self.start_menu, "Select Folder")
		if folder_path:
			self.project_folder.setText(folder_path)

	def create_project(self):
		self.start_menu.manifest.create_project(
			project_filepath=self.generated_project_filepath(),
			src_folder=self.generated_src_folder(),
			res_folder=self.generated_res_folder(),
			gen_folder=self.generated_gen_folder()
		)
		self.start_menu.open_project(self.generated_project_filepath())


class OpenTab:
	def __init__(self, start_menu: StartMenuWidget):
		self.start_menu = start_menu
		self.browse = start_menu.ui.openBrowseButton
		self.open_project_filepath = start_menu.ui.openProject
		self.open_project_button = start_menu.ui.openProjectButton

		self.browse.clicked.connect(self.open_browse)
		self.open_project_filepath.textChanged.connect(self.sync_project_filepath)

	def open_browse(self):
		filepath, _ = QFileDialog.getOpenFileName(self.start_menu, "Open Project", selectedFilter="*.oly")
		if filepath:
			if self.start_menu.manifest.is_valid_project_file(filepath):
				self.open_project_filepath.setText(filepath)

	def sync_project_filepath(self):
		disable = self.open_project_filepath.text() == ""
		self.open_project_button.setDisabled(disable)

	def open_project(self):
		self.start_menu.open_project(self.open_project_filepath.text())


class RecentTab:
	def __init__(self, start_menu: StartMenuWidget):
		self.start_menu = start_menu
		self.combo_box = start_menu.ui.recentCombo
		self.open_recent_button = start_menu.ui.openRecentProjectButton

		filepaths = start_menu.manifest.get_recent_project_filepaths()
		self.combo_box.addItems(filepaths)

		self.open_recent_button.clicked.connect(self.open_recent)

	def open_recent(self):
		filepath = self.combo_box.currentText()
		if filepath != "":
			self.start_menu.open_project(filepath)
