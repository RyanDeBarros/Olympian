from PySide6.QtGui import QShortcut, QKeySequence, QIcon
from PySide6.QtWidgets import QMainWindow, QWhatsThis

from editor.core.ProjectContext import ProjectContext


class MainWindow(QMainWindow):
	def __init__(self, open_start_menu, project_file):
		super().__init__()
		self.open_start = open_start_menu

		self.setWindowTitle("Olympian Editor")
		# TODO v4 create and set window icon

		self.project_context = ProjectContext(project_file, self)

		from editor import ui
		self.ui = ui.MainWindow.Ui_MainWindow()
		self.ui.setupUi(self)

		self.ui.actionOpen_Start_Menu.triggered.connect(self.open_start_menu)

		self.ui.actionContent_Browser.triggered.connect(self.open_content_browser)
		content_browser_shortcut = QShortcut(QKeySequence("Ctrl+Space"), self)
		content_browser_shortcut.activated.connect(self.open_content_browser)

		self.ui.actionProject_Settings.triggered.connect(self.open_project_settings)
		self.ui.actionAsset_Defaults.triggered.connect(self.open_asset_defaults)

		self.ui.actionEditor_Preferences.triggered.connect(self.open_editor_preferences)

		self.ui.actionOpen_Docs.triggered.connect(self.open_documentation)
		self.ui.actionHelp_Mode.triggered.connect(self.enter_help_mode)
		enter_help_mode_shortcut = QShortcut(QKeySequence("Ctrl+?"), self)
		enter_help_mode_shortcut.activated.connect(self.enter_help_mode)

		self.content_browser = self.ui.CBWidget
		self.content_browser.init(self)

		from editor.core.MainTabWidget import MainTabWidget
		self.tab_holder: MainTabWidget = self.ui.mainTabHolder
		self.tab_holder.init(self)

		self.ui.actionSave.triggered.connect(self.tab_holder.save)
		save_shortcut = QShortcut(QKeySequence("Ctrl+S"), self)
		save_shortcut.activated.connect(self.tab_holder.save)
		self.ui.actionSave_All.triggered.connect(self.tab_holder.save_all)
		save_all_shortcut = QShortcut(QKeySequence("Ctrl+Shift+S"), self)
		save_all_shortcut.activated.connect(self.tab_holder.save_all)
		self.ui.actionRevert_Changes.triggered.connect(self.tab_holder.revert_changes)
		revert_changes_shortcut = QShortcut(QKeySequence("Ctrl+Alt+Shift+R"), self)
		revert_changes_shortcut.activated.connect(self.tab_holder.revert_changes)

	def open_start_menu(self):
		self.close()
		self.open_start()

	def open_project_settings(self):
		pass  # TODO v3

	def open_content_browser(self):
		self.ui.contentBrowser.show()

	def open_asset_defaults(self):
		pass  # TODO v3

	def open_editor_preferences(self):
		pass  # TODO v3

	@staticmethod
	def open_documentation():
		pass  # TODO v4 open documentation webpage

	@staticmethod
	def enter_help_mode():
		QWhatsThis.enterWhatsThisMode()

	def open_standard_file(self, filepath, name, icon: QIcon):
		from .tabs import StandardFile
		tab = StandardFile(self, filepath, name)
		self.tab_holder.add_tab(filepath, tab, icon, name)
