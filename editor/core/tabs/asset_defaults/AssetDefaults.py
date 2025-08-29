from enum import IntEnum
from typing import override

from PySide6.QtCore import QSize

from editor import ui, TOMLAdapter
from editor.core import MainWindow, AbstractPathItem, nice_icon
from . import Defaults, Converters
from ..EditorTab import EditorTab
from ..asset_structures import Texture, Font
from ...common.SettingsForm import handle_all_children_modification


class TypeIndex(IntEnum):
	EMPTY = 0
	TEXTURE = 1
	FONT = 2


class AssetDefaultsTab(EditorTab):
	class UID:
		def __eq__(self, other):
			return isinstance(other, AssetDefaultsTab.UID)

	def __init__(self, win: MainWindow):
		super().__init__(win)

		self.defaults = Defaults.Defaults()
		self.directory = self.win.project_context.asset_defaults_directory

		self.ui = ui.AssetDefaults.Ui_AssetDefaults()
		self.ui.setupUi(self)

		self.ui.assetTypeSelect.currentIndexChanged.connect(self.select_asset_type)

		self.ui.fontCommonBuffer.editingFinished.connect(lambda: self.ui.fontCommonBuffer.clearFocus())

		self.revert_changes_impl()
		handle_all_children_modification(self.ui.stackedWidget, self.control_modified)

	def control_modified(self):
		self.set_asterisk(True)
		self.load_defaults_from_ui()

	@override
	def uid(self):
		return AssetDefaultsTab.UID()

	@override
	def icon(self, size: QSize):
		return nice_icon("res/images/Gear.png", size)

	@override
	def name(self):
		return "Asset Defaults"

	@override
	def save_changes_impl(self):
		self.load_defaults_from_ui()
		TOMLAdapter.dump(self.directory.texture_file, self.defaults.texture.to_dict(), {'type': 'texture_defaults'})
		TOMLAdapter.dump(self.directory.font_file, self.defaults.font.to_dict(), {'type': 'font_defaults'})

	@override
	def revert_changes_impl(self):
		self.defaults.texture = Texture.from_dict(TOMLAdapter.load(self.directory.texture_file))
		self.defaults.font = Font.from_dict(TOMLAdapter.load(self.directory.font_file))
		self.load_ui_from_defaults()

	@override
	def rename_impl(self, item: AbstractPathItem):
		raise RuntimeError("Asset defaults tab cannot be renamed")

	@override
	def refresh_impl(self):
		pass

	def select_asset_type(self):
		self.ui.stackedWidget.setCurrentIndex(self.ui.assetTypeSelect.currentIndex())
		self.load_ui_from_defaults()

	def load_ui_from_defaults(self):
		match self.ui.stackedWidget.currentIndex():
			case TypeIndex.TEXTURE:
				Converters.convert_to_ui_from_texture(self.ui, self.defaults.texture)
			case TypeIndex.FONT:
				Converters.convert_to_ui_from_font(self.ui, self.defaults.font)

	def load_defaults_from_ui(self):
		match self.ui.stackedWidget.currentIndex():
			case TypeIndex.TEXTURE:
				Converters.convert_to_texture_from_ui(self.ui, self.defaults.texture)
			case TypeIndex.FONT:
				Converters.convert_to_font_from_ui(self.ui, self.defaults.font)
