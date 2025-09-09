from __future__ import annotations

import os
from pathlib import Path
from typing import override, TYPE_CHECKING

from PySide6.QtCore import QSize

from .AbstractPathItem import AbstractPathItem
from .. import nice_icon

if TYPE_CHECKING:
	from ..content_browser import ContentBrowser


class StandardFilePathItem(AbstractPathItem):
	@override
	def icon(self, size: QSize):
		return nice_icon("res/images/File.png", size)

	@override
	def ui_name(self):
		return self.full_path.name

	@override
	def renamed_filepath(self, name: str):
		return self.full_path.parent / name

	@staticmethod
	def new_item(browser: ContentBrowser):
		file_name = "NewFile"
		i = 1
		while os.path.exists(os.path.join(browser.current_folder, file_name)):
			file_name = f"NewFile ({i})"
			i = i + 1
		file_path = os.path.join(browser.current_folder, file_name)
		browser.folder_view.file_machine.new_file(file_path)
		browser.folder_view.add_item(StandardFilePathItem((browser.current_folder / file_name).resolve()), editing=True)

	@override
	def open(self, browser: ContentBrowser):
		from ..tabs import StandardFileTab
		browser.win.tab_holder.add_tab(StandardFileTab(browser.win, self))

	@override
	def on_import(self, browser: ContentBrowser):
		from editor.core.EditorPreferences import PREFERENCES
		if self.full_path.suffix.lower() in PREFERENCES.texture_file_extensions:
			from . import ImportedTexturePathItem
			ImportedTexturePathItem(self.full_path).on_import(browser)
			browser.refresh_item(self)
		elif self.full_path.suffix.lower() in PREFERENCES.font_file_extensions:
			from . import ImportedFontPathItem
			ImportedFontPathItem(self.full_path).on_import(browser)
			browser.refresh_item(self)

	@override
	def on_new(self, browser: ContentBrowser):
		pass

	@override
	def on_delete(self, browser: ContentBrowser):
		pass

	@override
	def on_rename(self, browser: ContentBrowser, old_path: Path):
		pass
