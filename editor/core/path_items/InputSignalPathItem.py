from __future__ import annotations

import os
from pathlib import Path
from typing import override, TYPE_CHECKING

import toml
from PySide6.QtCore import QSize
from PySide6.QtGui import QIcon, QPixmap

from editor.core.path_items.AbstractPathItem import AbstractPathItem

if TYPE_CHECKING:
	from editor.core.ContentBrowser import ContentBrowser


class InputSignalPathItem(AbstractPathItem):
	@override
	def icon(self, size: QSize):
		return QIcon(QPixmap("res/images/InputSignal.png").scaled(size))

	@override
	def ui_name(self):
		return self.full_path.stem

	@override
	def renamed_filepath(self, name: str):
		return Path(str(self.full_path.parent.joinpath(name)) + '.oly')

	@override
	def new_item(self, browser: ContentBrowser):
		file_name = "NewSignal.oly"
		i = 1
		while os.path.exists(os.path.join(browser.current_folder, file_name)):
			file_name = f"NewSignal ({i}).oly"
			i = i + 1
		file_path = os.path.join(browser.current_folder, file_name)
		browser.folder_view.file_machine.new_file(file_path)
		item = InputSignalPathItem(parent_folder=browser.current_folder, name=file_name)
		browser.folder_view.add_item(item, editing=True)

		initial_data = {
			"header": "signal"
		}
		with open(item.full_path, 'w') as f:
			toml.dump(initial_data, f)

		# TODO v3 add signal to project file's list of input signals

	@override
	def open(self, browser: ContentBrowser):
		from ..tabs import InputSignalTab
		browser.win.tab_holder.add_tab(InputSignalTab(browser.win, self))
