from __future__ import annotations

import os
from pathlib import Path
from typing import override, TYPE_CHECKING

from PySide6.QtCore import QSize

from .AbstractPathItem import AbstractPathItem
from .. import nice_icon

if TYPE_CHECKING:
	from ..content_browser import ContentBrowser


class FolderPathItem(AbstractPathItem):
	@override
	def icon(self, size: QSize):
		return nice_icon("res/images/Folder.png", size)

	@override
	def ui_name(self):
		return self.full_path.name

	@override
	def renamed_filepath(self, name: str):
		return self.full_path.parent.joinpath(name)

	@override
	def sorting_key(self):
		if self.ui_name() == "..":
			return 0,
		else:
			return 1, self.ui_name().lower()

	@staticmethod
	def new_item(browser: ContentBrowser):
		folder_name = "NewFolder"
		i = 1
		while os.path.exists(os.path.join(browser.current_folder, folder_name)):
			folder_name = f"NewFolder ({i})"
			i = i + 1
		folder_path = os.path.join(browser.current_folder, folder_name)
		browser.folder_view.file_machine.new_folder(folder_path)
		browser.folder_view.add_item(FolderPathItem(browser.current_folder.joinpath(folder_name).resolve()),
									 editing=True)

	@override
	def on_new(self, browser: ContentBrowser):
		pass

	@override
	def on_import(self, browser: ContentBrowser):
		from . import get_path_item
		for path in self.full_path.iterdir():
			item = get_path_item(path)
			if item is not None:
				item.on_import(browser)

	@override
	def open(self, browser: ContentBrowser):
		browser.open_folder(self.full_path)

	@override
	def on_delete(self, browser: ContentBrowser):
		pass

	@override
	def on_rename(self, browser: ContentBrowser, old_path: Path):
		pass
