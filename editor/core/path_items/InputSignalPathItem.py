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

	@staticmethod
	def new_item(browser: ContentBrowser):
		file_name = "NewSignal.oly"
		i = 1
		while os.path.exists(browser.current_folder.joinpath(file_name).resolve()):
			file_name = f"NewSignal ({i}).oly"
			i = i + 1
		file_path = browser.current_folder.joinpath(file_name).resolve()

		initial_data = {
			"header": "signal"
		}
		with open(file_path, 'w') as f:
			toml.dump(initial_data, f)

		browser.folder_view.file_machine.new_file(file_path)
		item = InputSignalPathItem(file_path)
		browser.folder_view.add_item(item, editing=True)

	@override
	def open(self, browser: ContentBrowser):
		from ..tabs import InputSignalTab
		browser.win.tab_holder.add_tab(InputSignalTab(browser.win, self))

	@override
	def on_new(self, browser: ContentBrowser):
		project_context = browser.win.project_context
		project_context.input_signal_registry.add_signal_asset(self.full_path.relative_to(project_context.res_folder))

	@override
	def on_delete(self, browser: ContentBrowser):
		project_context = browser.win.project_context
		project_context.input_signal_registry.remove_signal_asset(self.full_path.relative_to(project_context.res_folder))

	@override
	def on_rename(self, browser: ContentBrowser, old_path: Path):
		project_context = browser.win.project_context
		project_context.input_signal_registry.rename_signal_asset(old_path.relative_to(project_context.res_folder),
																  self.full_path.relative_to(project_context.res_folder))
