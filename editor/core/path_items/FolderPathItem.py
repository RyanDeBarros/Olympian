from __future__ import annotations

import os
from typing import override, TYPE_CHECKING

from PySide6.QtCore import QSize
from PySide6.QtGui import QIcon, QPixmap

from editor.core.path_items.AbstractPathItem import AbstractPathItem

if TYPE_CHECKING:
	from editor.core.ContentBrowser import ContentBrowser


class FolderPathItem(AbstractPathItem):
	@override
	def icon(self, size: QSize):
		return QIcon(QPixmap("res/images/Folder.png").scaled(size))

	@override
	def ui_name(self):
		return self.name

	@override
	def renamed_filepath(self, name: str):
		return self.parent_folder.joinpath(name)

	@override
	def sorting_key(self):
		if self.ui_name() == "..":
			return 0,
		else:
			return 1, self.ui_name().lower()

	@override
	def new_item(self, browser: ContentBrowser):
		folder_name = "NewFolder"
		i = 1
		while os.path.exists(os.path.join(browser.current_folder, folder_name)):
			folder_name = f"NewFolder ({i})"
			i = i + 1
		folder_path = os.path.join(browser.current_folder, folder_name)
		browser.folder_view.file_machine.new_folder(folder_path)
		browser.folder_view.add_item(FolderPathItem(parent_folder=browser.current_folder, name=folder_name),
									 editing=True)

	@override
	def open(self, browser: ContentBrowser):
		browser.open_folder(self.full_path)
