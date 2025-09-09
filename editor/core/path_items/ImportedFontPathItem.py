from __future__ import annotations

from pathlib import Path
from typing import override, TYPE_CHECKING

from PySide6.QtCore import QSize

from .AbstractPathItem import AbstractPathItem
from .. import nice_icon
from ...tools import TOMLAdapter

if TYPE_CHECKING:
	from ..content_browser import ContentBrowser


class ImportedFontPathItem(AbstractPathItem):
	def __init__(self, full_path):
		super().__init__(full_path)
		self.import_path = Path(full_path.as_posix() + '.oly')

	@override
	def icon(self, size: QSize):
		return nice_icon("res/images/Font.png", size)

	@override
	def ui_name(self):
		return self.full_path.name

	@override
	def renamed_filepath(self, name: str):
		return self.full_path.parent.joinpath(name)

	@override
	def open(self, browser: ContentBrowser):
		from ..tabs import FontTab
		browser.win.tab_holder.add_tab(FontTab(browser.win, self))

	@override
	def on_import(self, browser: ContentBrowser):
		if not self.import_path.exists():
			self.import_path.touch()

		meta = TOMLAdapter.meta(self.import_path)
		if 'type' not in meta or meta['type'] != 'font':
			from ..tabs.asset_structures.Font import Font

			defaults = Font.from_dict(TOMLAdapter.load(browser.win.project_context.asset_defaults_directory.font_file))
			d: dict = {
				'font_face': defaults.to_dict(),
				'font_atlas': [defaults.font_atlas.to_dict()]
			}
			TOMLAdapter.dump(self.import_path, d, {'type': 'font'})

	@override
	def on_new(self, browser: ContentBrowser):
		pass

	@override
	def on_delete(self, browser: ContentBrowser):
		pass

	@override
	def on_rename(self, browser: ContentBrowser, old_path: Path):
		self.import_path = Path(self.full_path.as_posix() + '.oly')
