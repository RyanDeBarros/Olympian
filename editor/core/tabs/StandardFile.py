from PySide6.QtGui import QShortcut, Qt
from PySide6.QtWidgets import QWidget

from editor import ui
from editor.core import MainWindow, StandardFilePathItem


class StandardFile(QWidget):
	def __init__(self, win: MainWindow, item: StandardFilePathItem):
		super().__init__()
		self.win = win
		self.holder = self.win.tab_holder
		self.item = item

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
			self.holder.set_tab_name(self, self.item.ui_name())

	def revert_changes(self):
		self.load_file()
		if self.asterisk:
			self.asterisk = False
			self.holder.set_tab_name(self, self.item.ui_name())

	def load_file(self):
		with open(self.item.full_path, 'r') as f:
			self.text_edit.setPlainText(f.read())

	def save_file(self):
		with open(self.item.full_path, 'w') as f:
			f.write(self.text_edit.toPlainText())

	def rename(self, item: StandardFilePathItem):
		self.item = item
		self.holder.set_tab_name(self, f"* {self.item.ui_name()}" if self.asterisk else self.item.ui_name())

	def text_changed(self):
		if not self.asterisk:
			self.asterisk = True
			self.holder.set_tab_name(self, f"* {self.item.ui_name()}")
