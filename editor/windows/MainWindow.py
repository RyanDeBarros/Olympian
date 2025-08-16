from PySide6.QtCore import QKeyCombination
from PySide6.QtGui import QShortcut, QKeySequence, Qt
from PySide6.QtWidgets import QMainWindow, QWhatsThis

from editor import ui


class MainWindow(QMainWindow):
	def __init__(self, open_start_menu):
		super().__init__()
		self.open_start = open_start_menu

		self.setWindowTitle("Olympian Editor")
		# TODO v4 create and set window icon

		self.ui = ui.MainWindow.Ui_MainWindow()
		self.ui.setupUi(self)

		self.ui.actionOpen_Start_Menu.triggered.connect(self.open_start_menu)

		self.ui.actionContent_Browser.triggered.connect(self.open_content_browser)
		content_browser_shortcut = QShortcut(
			QKeySequence(QKeyCombination(Qt.KeyboardModifier.ControlModifier, Qt.Key.Key_Space)), self)
		content_browser_shortcut.activated.connect(self.open_content_browser)

		self.ui.actionProject_Settings.triggered.connect(self.open_project_settings)
		self.ui.actionAsset_Defaults.triggered.connect(self.open_asset_defaults)

		self.ui.actionOpen_Docs.triggered.connect(self.open_documentation)

		self.ui.actionHelp_Mode.triggered.connect(self.enter_help_mode)
		enter_help_mode_shortcut = QShortcut(
			QKeySequence(QKeyCombination(Qt.KeyboardModifier.ControlModifier, Qt.Key.Key_Question)), self)
		enter_help_mode_shortcut.activated.connect(self.enter_help_mode)

		self.content_browser = self.ui.CBWidget
		self.content_browser.win = self

	def open_start_menu(self):
		self.close()
		self.open_start()

	def open_project_settings(self):
		pass  # TODO v3

	def open_content_browser(self):
		self.ui.contentBrowser.show()

	def open_asset_defaults(self):
		pass  # TODO v3

	@staticmethod
	def open_documentation():
		pass  # TODO v4 open documentation webpage

	@staticmethod
	def enter_help_mode():
		QWhatsThis.enterWhatsThisMode()
