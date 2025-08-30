from typing import Optional

from PySide6.QtCore import QSize
from PySide6.QtGui import Qt
from PySide6.QtWidgets import QTabWidget, QMessageBox

from editor.core import MainWindow
from editor.core.tabs.EditorTab import EditorTab


class MainTabHolder(QTabWidget):
	def __init__(self, parent):
		super().__init__(parent)
		self.win: Optional[MainWindow] = None
		self.uids = []

		self.tabBar().tabMoved.connect(self.tab_moved)
		self.tabCloseRequested.connect(self.close_tab)
		self.tabBar().currentChanged.connect(self.tab_changed)

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
		tab = self.editor_tab_at(index)
		if tab.asterisk:
			reply = QMessageBox.question(self, f"{tab.name()} has unsaved changes",
										 "Do you want to save these changes?",
										 QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No | QMessageBox.StandardButton.Cancel,
										 QMessageBox.StandardButton.Cancel)
			if reply == QMessageBox.StandardButton.Cancel:
				return False
			if reply == QMessageBox.StandardButton.Yes:
				tab.save_changes()
		self.removeTab(index)
		self.uids.pop(index)
		if len(self.uids) == 0:
			self.win.enable_tab_menu_actions(False)
		return True

	def close_all(self):
		while len(self.uids) > 0:
			if not self.close_tab(0):
				return False
		self.win.enable_tab_menu_actions(False)
		return True

	def remove_tab(self, index):
		self.removeTab(index)
		self.uids.pop(index)
		if len(self.uids) == 0:
			self.win.enable_tab_menu_actions(False)

	def current_editor_tab(self):
		widget = self.currentWidget()
		assert isinstance(widget, EditorTab) or widget is None
		return widget

	def editor_tab_at(self, index):
		widget = self.widget(index)
		assert isinstance(widget, EditorTab) or widget is None
		return widget

	def add_tab(self, tab: EditorTab):
		uid = tab.uid()
		if uid in self.uids:
			self.setCurrentIndex(self.uids.index(uid))
		else:
			index = self.addTab(tab, tab.icon(QSize(64, 64)), tab.name())
			self.setCurrentIndex(index)
			self.uids.insert(index, uid)
		self.win.enable_tab_menu_actions(True)

	def save(self):
		tab = self.current_editor_tab()
		if tab is not None:
			tab.save_changes()

	def save_all(self):
		for i in range(self.count()):
			self.editor_tab_at(i).save_changes()

	def revert_changes(self):
		tab = self.current_editor_tab()
		if tab is not None:
			tab.revert_changes()

	def tab_changed(self):
		tab = self.current_editor_tab()
		if tab is not None:
			tab.refresh_impl()
