from abc import ABC, abstractmethod, ABCMeta

from PySide6.QtCore import QSize
from PySide6.QtWidgets import QWidget

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
