from pathlib import Path

from PySide6.QtCore import QEvent
from PySide6.QtGui import QShortcut, Qt, QWheelEvent
from PySide6.QtWidgets import QWidget, QTextEdit

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
		self.text_edit = self.ui.textEdit
		self.text_edit.init()

		lose_focus_shortcut = QShortcut(Qt.Key.Key_Escape, self)
		lose_focus_shortcut.activated.connect(self.lose_focus)

		self.load_file()
		self.text_edit.textChanged.connect(self.text_changed)

	def lose_focus(self):
		if self.text_edit.hasFocus():
			self.text_edit.clearFocus()

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
			self.text_edit.setPlainText(f.read())

	def save_file(self):
		with open(self.filepath, 'w') as f:
			f.write(self.text_edit.toPlainText())

	def text_changed(self):
		if not self.asterisk:
			self.asterisk = True
			self.holder.set_tab_name(self, f"* {self.name}")
