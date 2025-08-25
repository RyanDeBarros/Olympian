from __future__ import annotations

import os
from typing import override, TYPE_CHECKING

from PySide6.QtCore import QSize
from PySide6.QtGui import QIcon, QPixmap

from editor.core.path_items.AbstractPathItem import AbstractPathItem

if TYPE_CHECKING:
	from editor.core.ContentBrowser import ContentBrowser


class StandardFilePathItem(AbstractPathItem):
	@override
	def icon(self, size: QSize):
		return QIcon(QPixmap("res/images/File.png").scaled(size))

	@override
	def ui_name(self):
		return self.full_path.name

	@override
	def renamed_filepath(self, name: str):
		return self.full_path.parent.joinpath(name)

	@override
	def new_item(self, browser: ContentBrowser):
		file_name = "NewFile"
		i = 1
		while os.path.exists(os.path.join(browser.current_folder, file_name)):
			file_name = f"NewFile ({i})"
			i = i + 1
		file_path = os.path.join(browser.current_folder, file_name)
		browser.folder_view.file_machine.new_file(file_path)
		browser.folder_view.add_item(StandardFilePathItem(parent_folder=browser.current_folder, name=file_name),
									 editing=True)

	@override
	def open(self, browser: ContentBrowser):
		browser.win.open_standard_file(self)
