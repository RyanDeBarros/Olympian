from typing import override

from PySide6.QtCore import QSize

from editor.core import MainWindow, ImportedTexturePathItem
from .EditorTab import EditorTab


class TextureTab(EditorTab):
	def __init__(self, win: MainWindow, item: ImportedTexturePathItem):
		super().__init__(win)
		self.item = item

		# TODO v3 load UI

		self.revert_changes_impl()

	@override
	def uid(self):
		return self.item.full_path

	@override
	def icon(self, size: QSize):
		return self.item.icon(size)

	@override
	def name(self):
		return self.item.ui_name()

	@override
	def save_changes_impl(self):
		pass  # TODO v3

	@override
	def revert_changes_impl(self):
		pass  # TODo v3

	@override
	def rename_impl(self, item: ImportedTexturePathItem):
		assert isinstance(item, ImportedTexturePathItem)
		self.item = item

	@override
	def refresh_impl(self):
		pass  # TODO v3 update 'reset to default' button states next to fields
