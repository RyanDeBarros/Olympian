from typing import Optional

from PySide6.QtGui import QIcon, Qt
from PySide6.QtWidgets import QTabWidget, QWidget

from editor.core import MainWindow
from editor.util import FileIOMachine


class MainTabWidget(QTabWidget):
	def __init__(self, parent):
		super().__init__(parent)
		self.win: Optional[MainWindow] = None
		self.uids = []

		self.tabBar().tabMoved.connect(self.tab_moved)
		self.tabCloseRequested.connect(self.close_tab)

	def init(self, win):
		self.win = win

	def mouseReleaseEvent(self, event):
		if event.button() == Qt.MouseButton.MiddleButton:
			index = self.tabBar().tabAt(event.pos())
			self.close_tab(index)

	def tab_moved(self, from_index, to_index):
		uid = self.uids.pop(from_index)
		self.uids.insert(to_index, uid)

	def close_tab(self, index):
		# TODO v3 prompt user if unsaved changes
		self.removeTab(index)
		self.uids.pop(index)

	def add_tab(self, uid, tab: QWidget, icon: QIcon, name: str):
		if uid in self.uids:
			self.setCurrentIndex(self.uids.index(uid))
		else:
			index = self.addTab(tab, icon, name)
			self.setCurrentIndex(index)
			self.uids.insert(index, uid)

	def save(self):
		widget = self.currentWidget()
		if hasattr(widget, "save_changes") and callable(widget.save_changes):
			widget.save_changes()

	def save_all(self):
		for i in range(self.count()):
			widget = self.widget(i)
			if hasattr(widget, "save_changes") and callable(widget.save_changes):
				widget.save_changes()

	def revert_changes(self):
		widget = self.currentWidget()
		if hasattr(widget, "revert_changes") and callable(widget.revert_changes):
			widget.revert_changes()

	def set_tab_name(self, tab: QWidget, name: str):
		index = self.indexOf(tab)
		self.setTabText(index, name)
