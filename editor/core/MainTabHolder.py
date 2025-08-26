from abc import ABC, abstractmethod, ABCMeta
from typing import Optional

from PySide6.QtGui import QIcon, Qt
from PySide6.QtWidgets import QTabWidget, QWidget

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


# TODO v3 update tabs if files are renamed/moved/deleted
class MainTabHolder(QTabWidget):
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

	def current_editor_tab(self):
		widget = self.currentWidget()
		assert isinstance(widget, EditorTab)
		return widget

	def editor_tab_at(self, index):
		widget = self.widget(index)
		assert isinstance(widget, EditorTab)
		return widget

	def add_tab(self, uid, tab: QWidget, icon: QIcon, name: str):
		if uid in self.uids:
			self.setCurrentIndex(self.uids.index(uid))
		else:
			index = self.addTab(tab, icon, name)
			self.setCurrentIndex(index)
			self.uids.insert(index, uid)

	def save(self):
		self.current_editor_tab().save_changes()

	def save_all(self):
		for i in range(self.count()):
			self.editor_tab_at(i).save_changes()

	def revert_changes(self):
		self.current_editor_tab().revert_changes()
