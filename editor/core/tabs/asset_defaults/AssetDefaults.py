from typing import override

from PySide6.QtCore import QSize

from editor import ui, TOMLAdapter
from editor.core import MainWindow, AbstractPathItem, nice_icon
from editor.core.common.SettingsForm import handle_all_children_modification
from editor.core.tabs.EditorTab import EditorTab
from . import DefaultPODs


class AssetDefaultsTab(EditorTab):
	class UID:
		def __eq__(self, other):
			return isinstance(other, AssetDefaultsTab.UID)

	def __init__(self, win: MainWindow):
		super().__init__(win)

		self.defaults = DefaultPODs.Defaults()
		self.filepath = self.win.project_context.asset_defaults_file

		self.ui = ui.AssetDefaults.Ui_AssetDefaults()
		self.ui.setupUi(self)

		self.ui.assetTypeSelect.currentIndexChanged.connect(self.select_asset_type)

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
		TOMLAdapter.dump(self.filepath, self.defaults.to_dict(), {'type': 'asset_defaults'})

	@override
	def revert_changes_impl(self):
		self.defaults = DefaultPODs.Defaults.from_dict(TOMLAdapter.load(self.filepath))
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
		pass  # TODO v3

	def load_defaults_from_ui(self):
		pass  # TODO v3
