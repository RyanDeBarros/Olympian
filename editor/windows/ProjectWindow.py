from PySide6.QtCore import QKeyCombination, Qt
from PySide6.QtGui import QShortcut, QKeySequence
from PySide6.QtWidgets import QMainWindow, QWhatsThis

from editor import ui
from editor.subeditors import *


# TODO v3 use watchdog to auto-import assets
class ProjectWindow(QMainWindow):
	def __init__(self, open_start_menu):
		super().__init__()
		self.open_start = open_start_menu

		self.setWindowTitle("Olympian Editor")
		# LATER create and set window icon

		self.ui = ui.ProjectWindow.Ui_MainWindow()
		self.ui.setupUi(self)

		self.ui.actionOpen_Start_Menu.triggered.connect(self.open_start_menu)
		self.ui.actionProjectSettings.triggered.connect(self.open_project_settings)

		self.ui.actionTexture.triggered.connect(self.open_texture_editor)
		self.ui.actionFont.triggered.connect(self.open_font_editor)

		self.ui.actionEnter_Help_Mode.triggered.connect(self.enter_help_mode)
		enter_help_mode_shortcut = QShortcut(
			QKeySequence(QKeyCombination(Qt.KeyboardModifier.ControlModifier, Qt.Key.Key_Question)), self)
		enter_help_mode_shortcut.activated.connect(self.enter_help_mode)
		self.ui.actionDocumentation.triggered.connect(self.open_documentation)

		self.ui.actionProjectSettings.trigger()

	def open_start_menu(self):
		self.close()
		self.open_start()

	@staticmethod
	def enter_help_mode():
		QWhatsThis.enterWhatsThisMode()

	@staticmethod
	def open_documentation():
		pass  # TODO v4 open documentation webpage

	def open_project_settings(self):
		self.setCentralWidget(ProjectSettingsWidget(self))

	def open_texture_editor(self):
		self.setCentralWidget(TextureEditorWidget(self))

	def open_font_editor(self):
		self.setCentralWidget(FontEditorWidget(self))
