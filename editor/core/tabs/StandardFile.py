from pathlib import Path

from PySide6.QtWidgets import QWidget

from editor import ui
from editor.core import MainWindow


class StandardFile(QWidget):
	def __init__(self, win: MainWindow, filepath: Path, name: str):
		super().__init__()
		self.win = win
		self.holder = self.win.tab_holder
		self.filepath = filepath
		self.name = name

		self.asterisk = False

		self.ui = ui.StandardFile.Ui_StandardFile()
		self.ui.setupUi(self)

		self.load_file()
		self.ui.textEdit.textChanged.connect(self.text_changed)

	def save_changes(self):
		self.save_file()
		if self.asterisk:
			self.asterisk = False
			self.holder.set_tab_name(self, self.name)

	def revert_changes(self):
		self.load_file()
		if self.asterisk:
			self.asterisk = False
			self.holder.set_tab_name(self, self.name)

	def load_file(self):
		with open(self.filepath, 'r') as f:
			self.ui.textEdit.setPlainText(f.read())

	def save_file(self):
		with open(self.filepath, 'w') as f:
			f.write(self.ui.textEdit.toPlainText())

	def text_changed(self):
		if not self.asterisk:
			self.asterisk = True
			self.holder.set_tab_name(self, f"* {self.name}")
