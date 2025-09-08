import os
import pathlib
import posixpath
import re

from PySide6.QtCore import QSize
from PySide6.QtWidgets import QWidget, QFileDialog, QMessageBox, QMainWindow

from editor import MANIFEST, ui


class StartMenuWindow(QMainWindow):
	def __init__(self, open_project):
		super().__init__()
		self.open_project = open_project

		self.setWindowTitle("Olympian Editor")
		self.setMinimumSize(QSize(600, 300))

		self.setCentralWidget(StartMenuWidget(self))

	def open(self, project_filepath):
		self.close()
		self.open_project(project_filepath)


class StartMenuWidget(QWidget):
	def __init__(self, win):
		super().__init__()
		self.win = win
		self.ui = ui.StartMenu.Ui_Form()
		self.ui.setupUi(self)

		self.new_tab = NewTab(self)
		self.open_tab = OpenTab(self)
		self.recent_tab = RecentTab(self)
		self.delete_tab = DeleteTab(self)

		self.ui.tabWidget.currentChanged.connect(self.on_tab_changed)
		self.recent_tab.sync_combo()

	def on_tab_changed(self):
		if self.ui.tabWidget.currentWidget() == self.ui.recentTab:
			self.recent_tab.sync_combo()

	def open_project(self, project_filepath):
		MANIFEST.push_to_top_of_recent(project_filepath)
		self.close()
		self.win.open(project_filepath)


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

	def get_project_name(self):
		return self.project_name.text().strip()

	def generated_project_folder(self):
		project = self.get_project_name()
		folder = self.project_folder.text()
		if NewTab.is_valid_project_name(project) and len(folder) > 0:
			if self.create_project_folder.isChecked():
				folder = posixpath.join(folder, project)
			return folder
		else:
			return ""

	def generated_project_filepath(self):
		project_folder = self.generated_project_folder()
		project_name = self.get_project_name()
		if project_folder != "" and project_name != "":
			return MANIFEST.get_project_file(project_folder, project_name)
		else:
			return ""

	def sync_project_name(self):
		project_folder = self.generated_project_folder()
		self.src_folder.setText(MANIFEST.get_src_folder(project_folder))
		self.res_folder.setText(MANIFEST.get_res_folder(project_folder))
		self.gen_folder.setText(MANIFEST.get_gen_folder(project_folder))
		project_filepath = self.generated_project_filepath()
		self.project_filepath.setText(project_filepath)

		disable = project_filepath == ""
		if disable:
			self.error_message.setText("")
		else:
			disable = MANIFEST.is_filepath_relative_to_existing_project(project_filepath)
			if disable:
				self.error_message.setText("The project filepath is relative to an existing project!")
			else:
				self.error_message.setText("")
		self.create_project_button.setDisabled(disable)

	def open_browse(self):
		folder_path = QFileDialog.getExistingDirectory(self.start_menu, "Select Folder",
													   MANIFEST.get_last_file_dialog_dir())
		if folder_path:
			MANIFEST.set_last_file_dialog_dir(folder_path)
			self.project_folder.setText(folder_path)

	def create_project(self):
		project_filepath = self.generated_project_filepath()
		MANIFEST.create_project(project_filepath)
		self.start_menu.open_project(project_filepath)


class OpenTab:
	def __init__(self, start_menu: StartMenuWidget):
		self.start_menu = start_menu
		self.browse = start_menu.ui.openBrowseButton
		self.open_project_filepath = start_menu.ui.openProject
		self.open_project_button = start_menu.ui.openProjectButton

		self.browse.clicked.connect(self.open_browse)
		self.open_project_filepath.textChanged.connect(self.sync_project_filepath)
		self.open_project_button.clicked.connect(self.open_project)

	def open_browse(self):
		filepath, _ = QFileDialog.getOpenFileName(self.start_menu, "Open Project", MANIFEST.get_last_file_dialog_dir(),
												  filter="Oly files (*.oly)")
		if filepath and MANIFEST.is_valid_project_file(filepath):
			MANIFEST.set_last_file_dialog_dir(os.path.dirname(filepath))
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

		self.open_recent_button.clicked.connect(self.open_recent)

	def open_recent(self):
		filepath = self.combo_box.currentText()
		if filepath != "":
			self.start_menu.open_project(filepath)

	def sync_combo(self):
		self.combo_box.clear()
		filepaths = MANIFEST.get_recent_project_filepaths()
		self.combo_box.addItems(filepaths)


class DeleteTab:
	def __init__(self, start_menu: StartMenuWidget):
		self.start_menu = start_menu
		self.browse = start_menu.ui.deleteBrowseButton
		self.delete_project_filepath = start_menu.ui.deleteProject
		self.delete_project_button = start_menu.ui.deleteProjectButton
		self.clean_manifest_button = start_menu.ui.cleanManifestButton

		self.browse.clicked.connect(self.open_browse)
		self.delete_project_filepath.textChanged.connect(self.sync_project_filepath)
		self.delete_project_button.clicked.connect(self.delete_project)
		self.clean_manifest_button.clicked.connect(self.clean_manifest)

	def open_browse(self):
		filepath, _ = QFileDialog.getOpenFileName(self.start_menu, "Open Project", MANIFEST.get_last_file_dialog_dir(), filter="Oly files (*.oly)")
		if filepath and MANIFEST.is_valid_project_file(filepath):
			MANIFEST.set_last_file_dialog_dir(os.path.dirname(filepath))
			self.delete_project_filepath.setText(filepath)

	def sync_project_filepath(self):
		disable = self.delete_project_filepath.text() == ""
		self.delete_project_button.setDisabled(disable)

	def delete_project(self):
		reply = QMessageBox.warning(self.start_menu, "Confirm",
									f"Delete project {pathlib.Path(self.delete_project_filepath.text()).stem}?",
									QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No)
		if reply == QMessageBox.StandardButton.Yes:
			MANIFEST.delete_project(self.delete_project_filepath.text())
			self.delete_project_filepath.clear()

	def clean_manifest(self):
		current_to_delete = self.delete_project_filepath.text()
		removed = MANIFEST.remove_nonexistent_projects()
		if current_to_delete in removed:
			self.delete_project_filepath.clear()
