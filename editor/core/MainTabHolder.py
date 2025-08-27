from abc import ABC, abstractmethod, ABCMeta
from typing import Optional

from PySide6.QtCore import QSize
from PySide6.QtGui import Qt
from PySide6.QtWidgets import QTabWidget, QWidget, QMessageBox

from editor.core import MainWindow, AbstractPathItem


class MetaEditorTab(type(QWidget), ABCMeta):
	pass


class EditorTab(QWidget, ABC, metaclass=MetaEditorTab):
	def __init__(self, win: MainWindow):
		super().__init__()
		self.win = win
		self.holder = self.win.tab_holder

		self.asterisk = False

	@abstractmethod
	def uid(self):
		pass

	@abstractmethod
	def icon(self, size: QSize):
		pass

	@abstractmethod
	def name(self):
		pass

	def asterisk_name(self):
		return f"* {self.name()}"

	@abstractmethod
	def rename_impl(self, item: AbstractPathItem):
		pass

	def rename(self, item: AbstractPathItem):
		self.rename_impl(item)
		self.rename_in_holder()

	def set_asterisk(self, asterisk: bool):
		if self.asterisk != asterisk:
			self.asterisk = asterisk
			self.rename_in_holder()

	def rename_in_holder(self):
		index = self.holder.indexOf(self)
		self.holder.setTabText(index, self.asterisk_name() if self.asterisk else self.name())

	@abstractmethod
	def save_changes_impl(self):
		pass

	def save_changes(self):
		self.save_changes_impl()
		self.set_asterisk(False)

	@abstractmethod
	def revert_changes_impl(self):
		pass

	def revert_changes(self):
		self.revert_changes_impl()
		self.set_asterisk(False)

	@abstractmethod
	def refresh_impl(self):
		pass


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

	# TODO if there are unsaved changes when editor is requested to close, prompt user
	def close_tab(self, index):
		tab = self.editor_tab_at(index)
		if tab.asterisk:
			reply = QMessageBox.question(self, f"{tab.name()} has unsaved changes", "Do you want to save these changes?",
										 QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No | QMessageBox.StandardButton.Cancel,
										 QMessageBox.StandardButton.Cancel)
			if reply == QMessageBox.StandardButton.Cancel:
				return
			if reply == QMessageBox.StandardButton.Yes:
				tab.save_changes()
		self.removeTab(index)
		self.uids.pop(index)

	def remove_tab(self, index):
		self.removeTab(index)
		self.uids.pop(index)

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

	def save(self):
		self.current_editor_tab().save_changes()

	def save_all(self):
		for i in range(self.count()):
			self.editor_tab_at(i).save_changes()

	def revert_changes(self):
		self.current_editor_tab().revert_changes()

	def tab_changed(self):
		self.current_editor_tab().refresh_impl()
