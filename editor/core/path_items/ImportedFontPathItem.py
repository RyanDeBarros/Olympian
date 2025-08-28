from __future__ import annotations

from pathlib import Path
from typing import override, TYPE_CHECKING

from PySide6.QtCore import QSize

from .AbstractPathItem import AbstractPathItem
from .. import nice_icon

if TYPE_CHECKING:
	from editor.core.ContentBrowser import ContentBrowser


# TODO v3 context menu option when right-clicking to re-import

class ImportedFontPathItem(AbstractPathItem):
	def __init__(self, full_path):
		super().__init__(full_path)
		self.import_path = Path(str(full_path) + '.oly')

	@override
	def icon(self, size: QSize):
		return nice_icon("res/images/File.png", size)  # TODO v3 font icon

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
	def on_new(self, browser: ContentBrowser):
		pass  # TODO v3 create import file if it doesn't exist - if on_new is called on a file being moved from trash, bring import file back from trash as well instead of creating new import file

	@override
	def on_delete(self, browser: ContentBrowser):
		pass  # TODO v3 delete import file if it doesn't exist -> move to trash folder where texture went

	@override
	def on_rename(self, browser: ContentBrowser, old_path: Path):
		self.import_path = Path(str(self.full_path) + '.oly')
