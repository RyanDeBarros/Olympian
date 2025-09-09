from __future__ import annotations

from pathlib import Path
from typing import override, TYPE_CHECKING

from PySide6.QtCore import QSize

from .AbstractPathItem import AbstractPathItem
from .. import nice_icon
from ...tools import TOMLAdapter

if TYPE_CHECKING:
	from editor.core.ContentBrowser import ContentBrowser


class ImportedTexturePathItem(AbstractPathItem):
	def __init__(self, full_path):
		super().__init__(full_path)
		self.import_path = Path(full_path.as_posix() + '.oly')

	@override
	def icon(self, size: QSize):
		return nice_icon(self.full_path, size)

	@override
	def ui_name(self):
		return self.full_path.name

	@override
	def renamed_filepath(self, name: str):
		return self.full_path.parent.joinpath(name)

	@override
	def open(self, browser: ContentBrowser):
		from ..tabs import TextureTab
		browser.win.tab_holder.add_tab(TextureTab(browser.win, self))

	@override
	def on_import(self, browser: ContentBrowser):
		if not self.import_path.exists():
			self.import_path.touch()

		meta = TOMLAdapter.meta(self.import_path)
		if 'type' not in meta or meta['type'] != 'texture':
			from ..tabs.texture.Converters import TextureSlot
			from ..tabs.asset_structures.Texture import Texture

			is_raster = self.full_path.suffix not in ('.svg', '.SVG')
			is_gif = self.full_path.suffix in ('.gif', '.GIF')
			slot = TextureSlot(is_raster, is_gif)
			defaults = Texture.from_dict(
				TOMLAdapter.load(browser.win.project_context.asset_defaults_directory.texture_file))
			slot.load_dict(defaults.to_dict())
			textures = [slot.get_dict()]

			content: dict = {'texture': textures}
			if not is_raster:
				content['abstract_storage'] = slot.svg.abstract_storage
			TOMLAdapter.dump(self.import_path, content, {'type': 'texture'})

	@override
	def on_new(self, browser: ContentBrowser):
		pass

	@override
	def on_delete(self, browser: ContentBrowser):
		pass

	@override
	def on_rename(self, browser: ContentBrowser, old_path: Path):
		self.import_path = Path(self.full_path.as_posix() + '.oly')
