import subprocess
from pathlib import Path

from PySide6.QtGui import QShortcut, QKeySequence, QIcon
from PySide6.QtWidgets import QMainWindow, QWhatsThis


class MainWindow(QMainWindow):
	def __init__(self, open_start_menu, project_file: Path):
		super().__init__()
		self.open_start = open_start_menu

		from editor.core.ProjectContext import ProjectContext
		self.project_context = ProjectContext(project_file, self)

		from editor import ui
		self.ui = ui.MainWindow.Ui_MainWindow()
		self.ui.setupUi(self)

		self.setWindowTitle(f"Olympian Editor - {self.project_context.project_file.stem}")

		self.ui.actionOpen_Start_Menu.triggered.connect(self.open_start_menu)

		self.ui.actionContent_Browser.triggered.connect(self.open_content_browser)
		self.ui.actionContent_Browser.setIcon(QIcon("res/images/Folder.png"))
		content_browser_shortcut = QShortcut(QKeySequence("Ctrl+Space"), self)
		content_browser_shortcut.activated.connect(self.open_content_browser)

		self.ui.actionProject_Settings.triggered.connect(self.open_project_settings)
		self.ui.actionProject_Settings.setIcon(QIcon("res/images/Gear.png"))
		self.ui.actionAsset_Defaults.triggered.connect(self.open_asset_defaults)
		self.ui.actionAsset_Defaults.setIcon(QIcon("res/images/Gear.png"))

		self.ui.actionEditor_Preferences.triggered.connect(self.open_editor_preferences)
		self.ui.actionEditor_Preferences.setIcon(QIcon("res/images/Gear.png"))

		self.ui.actionOpen_Docs.triggered.connect(self.open_documentation)
		self.ui.actionHelp_Mode.triggered.connect(self.enter_help_mode)
		enter_help_mode_shortcut = QShortcut(QKeySequence("Ctrl+?"), self)
		enter_help_mode_shortcut.activated.connect(self.enter_help_mode)

		self.content_browser = self.ui.CBWidget
		from editor.core.MainTabHolder import MainTabHolder
		self.tab_holder: MainTabHolder = self.ui.mainTabHolder

		self.content_browser.init(self)
		self.tab_holder.init(self)
		self.resize(1440, 1080)

		self.ui.actionSave.triggered.connect(self.tab_holder.save)
		self.ui.actionSave.setIcon(QIcon("res/images/Save.png"))
		save_shortcut = QShortcut(QKeySequence("Ctrl+S"), self)
		save_shortcut.activated.connect(self.tab_holder.save)
		self.ui.actionSave_All.triggered.connect(self.tab_holder.save_all)
		self.ui.actionSave_All.setIcon(QIcon("res/images/SaveAll.png"))
		save_all_shortcut = QShortcut(QKeySequence("Ctrl+Shift+S"), self)
		save_all_shortcut.activated.connect(self.tab_holder.save_all)
		self.ui.actionRevert_Changes.triggered.connect(self.tab_holder.revert_changes)
		self.ui.actionRevert_Changes.setIcon(QIcon("res/images/Undo.png"))
		revert_changes_shortcut = QShortcut(QKeySequence("Ctrl+Alt+Shift+R"), self)
		revert_changes_shortcut.activated.connect(self.tab_holder.revert_changes)
		self.enable_tab_menu_actions(False)

	def closeEvent(self, event):
		if self.tab_holder.close_all():
			event.accept()
		else:
			event.ignore()

	def enable_tab_menu_actions(self, enable: bool):
		self.ui.actionSave.setEnabled(enable)
		self.ui.actionSave_All.setEnabled(enable)
		self.ui.actionRevert_Changes.setEnabled(enable)

	def open_start_menu(self):
		self.close()
		self.open_start()

	def open_project_settings(self):
		from .tabs import ProjectSettingsTab
		self.tab_holder.add_tab(ProjectSettingsTab(self))

	def open_content_browser(self):
		self.ui.contentBrowser.show()

	def open_asset_defaults(self):
		from .tabs import AssetDefaultsTab
		self.tab_holder.add_tab(AssetDefaultsTab(self))

	def open_editor_preferences(self):
		from .tabs import EditorPreferencesTab
		self.tab_holder.add_tab(EditorPreferencesTab(self))

	@staticmethod
	def open_documentation():
		subprocess.run(["python", "Serve.py"], cwd="../mkdocs")

	@staticmethod
	def enter_help_mode():
		QWhatsThis.enterWhatsThisMode()

	@staticmethod
	def quit_project(code):
		exit(code)
