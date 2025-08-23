from typing import Optional

from PySide6.QtGui import QIcon
from PySide6.QtWidgets import QTabWidget, QWidget

from editor.core import MainWindow
from editor.util import FileIOMachine


class MainTabWidget(QTabWidget):
	def __init__(self, parent):
		super().__init__(parent)
		self.win: Optional[MainWindow] = None
		self.file_machine: Optional[FileIOMachine] = None
		self.uids = []

		self.tabBar().tabMoved.connect(self.tab_moved)
		self.tabCloseRequested.connect(self.close_tab)

	def init(self, win):
		self.win = win
		self.file_machine = self.win.project_context.file_machine

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
