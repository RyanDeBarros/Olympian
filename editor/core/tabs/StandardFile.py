from PySide6.QtWidgets import QWidget

from editor import ui
from editor.core import MainWindow


class StandardFile(QWidget):
	def __init__(self, win: MainWindow, filepath):
		super().__init__()
		self.win = win
		self.filepath = filepath

		self.ui = ui.StandardFile.Ui_StandardFile()
		self.ui.setupUi(self)
