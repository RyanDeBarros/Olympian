from __future__ import annotations

from pathlib import Path
from typing import override, TYPE_CHECKING

from PySide6.QtCore import QSize

from .AbstractPathItem import AbstractPathItem
from .. import nice_icon
from ...tools import TOMLAdapter

if TYPE_CHECKING:
	from ..content_browser import ContentBrowser


class InputSignalPathItem(AbstractPathItem):
	@override
	def icon(self, size: QSize):
		return nice_icon("res/images/InputSignal.png", size)

	@override
	def ui_name(self):
		return self.full_path.stem

	@override
	def renamed_filepath(self, name: str):
		return Path((self.full_path.parent / name).as_posix() + '.oly')

	@staticmethod
	def new_item(browser: ContentBrowser):
		file_name = "NewSignal.oly"
		i = 1
		while (browser.current_folder / file_name).exists():
			file_name = f"NewSignal ({i}).oly"
			i = i + 1
		file_path = (browser.current_folder / file_name).resolve()

		meta = {
			'type': 'signal'
		}
		TOMLAdapter.dump(file_path, {}, meta)

		browser.folder_view.file_machine.new_file(file_path)
		item = InputSignalPathItem(file_path)
		browser.folder_view.add_item(item, editing=True)

	@override
	def open(self, browser: ContentBrowser):
		from ..tabs import InputSignalTab
		browser.win.tab_holder.add_tab(InputSignalTab(browser.win, self))

	@staticmethod
	def _registry(browser: ContentBrowser):
		return browser.win.project_context.input_signal_registry

	def _asset_path(self, browser: ContentBrowser, path=None):
		if path is None:
			path = self.full_path
		return path.relative_to(browser.win.project_context.res_folder)

	@override
	def on_import(self, browser: ContentBrowser):
		if not self._registry(browser).has_signal_asset(self._asset_path(browser)):
			self._registry(browser).add_signal_asset(self._asset_path(browser))

	@override
	def on_new(self, browser: ContentBrowser):
		self._registry(browser).add_signal_asset(self._asset_path(browser))

	@override
	def on_delete(self, browser: ContentBrowser):
		self._registry(browser).remove_signal_asset(self._asset_path(browser))

	@override
	def on_rename(self, browser: ContentBrowser, old_path: Path):
		self._registry(browser).rename_signal_asset(self._asset_path(browser, old_path), self._asset_path(browser))
