from typing import override

from PySide6.QtGui import QShortcut, Qt

from editor import ui
from editor.core import MainWindow, StandardFilePathItem
from editor.core.MainTabHolder import EditorTab


class StandardFile(EditorTab):
	def __init__(self, win: MainWindow, item: StandardFilePathItem):
		super().__init__(win)
		self.item = item

		self.ui = ui.StandardFile.Ui_StandardFile()
		self.ui.setupUi(self)
		self.text_edit = self.ui.textEdit
		self.text_edit.init()

		lose_focus_shortcut = QShortcut(Qt.Key.Key_Escape, self)
		lose_focus_shortcut.activated.connect(self.lose_focus)

		self.load_file()
		self.text_edit.textChanged.connect(self.text_changed)

	@override
	def name(self):
		return self.item.ui_name()

	@override
	def save_changes_impl(self):
		self.save_file()

	@override
	def revert_changes_impl(self):
		self.load_file()

	def lose_focus(self):
		if self.text_edit.hasFocus():
			self.text_edit.clearFocus()

	def load_file(self):
		with open(self.item.full_path, 'r') as f:
			self.text_edit.setPlainText(f.read())

	def save_file(self):
		with open(self.item.full_path, 'w') as f:
			f.write(self.text_edit.toPlainText())

	@override
	def rename_impl(self, item: StandardFilePathItem):
		assert isinstance(item, StandardFilePathItem)
		self.item = item

	def text_changed(self):
		self.set_asterisk(True)
