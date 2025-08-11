from PySide6.QtWidgets import QMainWindow

from editor import ui
from editor import asset_editors


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

		self.ui.actionTexture.triggered.connect(self.open_texture_editor)
		self.ui.actionFont.triggered.connect(self.open_font_editor)

	def open_start_menu(self):
		self.close()
		self.open_start()

	def open_texture_editor(self):
		self.setCentralWidget(asset_editors.TextureEditorWidget(self))

	def open_font_editor(self):
		self.setCentralWidget(asset_editors.FontEditorWidget(self))
