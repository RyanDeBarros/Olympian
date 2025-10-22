from typing import override, Optional

from PySide6.QtCore import QSize
from PySide6.QtGui import QPixmap, QIcon
from PySide6.QtWidgets import QWidget, QPushButton, QFrame

from editor import ui
from editor.tools import TOMLAdapter
from editor.core import MainWindow, ImportedTexturePathItem, nice_pixmap, block_signals, PARAM_LIST
from . import Converters
from .Converters import TextureSlot
from ..EditorTab import EditorTab
from ..asset_structures import Texture
from ...common.SettingsForm import handle_all_children_modification, SettingsForm


# TODO v6 add option to view spritesheet when editing spritesheet parameters - in preview image, separate slong axes.

class TextureTab(EditorTab):
	def __init__(self, win: MainWindow, item: ImportedTexturePathItem):
		super().__init__(win)
		self.item = item
		self.item.import_path.touch()

		self.is_raster = self.item.full_path.suffix not in ('.svg', '.SVG')
		self.is_gif = self.item.full_path.suffix in ('.gif', '.GIF')
		self.scratch_textures: list[TextureSlot] = []
		self.scratch_abstract_storage: Optional[str] = ""

		self.ui = ui.Texture.Ui_Texture()
		self.ui.setupUi(self)

		self.ui.stackedWidget.setFrameShape(QFrame.Shape.NoFrame)

		if self.is_raster:
			self.ui.stackedWidget.setCurrentWidget(self.ui.rasterPage)
		else:
			self.ui.stackedWidget.setCurrentWidget(self.ui.svgPage)
		self.ui.spritesheetSettings.setVisible(not self.is_gif)
		self.setup_preview()

		self.ui.textureSlotSelect.currentIndexChanged.connect(self.select_texture_slot)
		self.ui.newTextureSlot.clicked.connect(self.new_texture_slot)
		self.ui.deleteTextureSlot.clicked.connect(self.delete_texture_slot)
		self.setup_reset_callbacks()

		self.ui.createSpritesheet.checkStateChanged.connect(self.spritesheet_toggled)
		self.spritesheet_toggled()

		self.revert_changes_impl()

		handle_all_children_modification(self.ui.stackedWidget, self.control_modified)
		handle_all_children_modification(self.ui.spritesheetSettings, self.control_modified)
		self.ui.newTextureSlot.clicked.connect(self.control_modified)
		self.ui.deleteTextureSlot.clicked.connect(self.control_modified)
		self.update_reset_states()

	@override
	def uid(self):
		return self.item.full_path

	@override
	def icon(self, size: QSize):
		return self.item.icon(size)

	@override
	def name(self):
		return self.item.ui_name()

	@override
	def save_changes_impl(self):
		textures = []
		for texture in self.scratch_textures:
			textures.append(texture.get_dict())

		content: dict = {'texture': textures}
		if not self.is_raster:
			content['abstract_storage'] = self.scratch_abstract_storage
		TOMLAdapter.dump(self.item.import_path, content, {'type': 'texture'})

	@override
	def revert_changes_impl(self):
		content = TOMLAdapter.load(self.item.import_path)
		if not self.is_raster:
			self.scratch_abstract_storage = content['abstract_storage']
		textures = content['texture']
		self.scratch_textures.clear()
		for texture in textures:
			if not self.is_raster:
				texture['abstract_storage'] = self.scratch_abstract_storage
			slot = TextureSlot(self.is_raster, self.is_gif)
			slot.load_dict(texture)
			self.scratch_textures.append(slot)

		with block_signals(self.ui.textureSlotSelect) as textureSlotSelect:
			textureSlotSelect.clear()
			for i in range(len(self.scratch_textures)):
				textureSlotSelect.addItem(f"{i}")
			textureSlotSelect.setCurrentIndex(0)
		self.select_texture_slot()

	@override
	def rename_impl(self, item: ImportedTexturePathItem):
		assert isinstance(item, ImportedTexturePathItem)
		self.item = item

	@override
	def refresh_impl(self):
		self.setup_preview()
		self.update_reset_states()

	def setup_preview(self):
		pixmap = QPixmap(self.item.full_path)
		if pixmap.width() == 0 or pixmap.height() == 0:
			return

		max_size = self.ui.texturePreview.maximumSize()
		self.ui.texturePreview.setPixmap(nice_pixmap(pixmap, max_size))

	def control_modified(self):
		self.set_asterisk(True)
		self.convert_to_texture_from_ui()
		self.update_reset_states()

	def get_defaults(self):
		return Texture.from_dict(TOMLAdapter.load(self.win.project_context.asset_defaults_directory.texture_file))

	@staticmethod
	def _setup_reset_callback(control: QWidget, reset_button: QPushButton, default):
		def callback():
			SettingsForm.set_value(control, default())
		reset_button.pressed.connect(lambda: callback())

		reset_button.setMaximumWidth(30)
		reset_button.setText("")
		reset_button.setIcon(QIcon("res/images/Undo.png"))
		policy = reset_button.sizePolicy()
		policy.setRetainSizeWhenHidden(True)
		reset_button.setSizePolicy(policy)

	def setup_reset_callbacks(self):
		if self.is_raster:
			self._setup_reset_callback(self.ui.textureRasterStorage, self.ui.textureRasterStorageReset, lambda: self.get_defaults().raster.storage)
			self._setup_reset_callback(self.ui.textureRasterMipmaps, self.ui.textureRasterMipmapsReset, lambda: self.get_defaults().raster.generate_mipmaps)
			self._setup_reset_callback(self.ui.textureRasterMinFilter, self.ui.textureRasterMinFilterReset, lambda: self.get_defaults().raster.min_filter)
			self._setup_reset_callback(self.ui.textureRasterMagFilter, self.ui.textureRasterMagFilterReset, lambda: self.get_defaults().raster.mag_filter)
			self._setup_reset_callback(self.ui.textureRasterWrapS, self.ui.textureRasterWrapSReset, lambda: self.get_defaults().raster.wrap_s)
			self._setup_reset_callback(self.ui.textureRasterWrapT, self.ui.textureRasterWrapTReset, lambda: self.get_defaults().raster.wrap_t)
		else:
			self._setup_reset_callback(self.ui.textureSVGImageStorage, self.ui.textureSVGImageStorageReset, lambda: self.get_defaults().svg.image_storage)
			self._setup_reset_callback(self.ui.textureSVGAbstractStorage, self.ui.textureSVGAbstractStorageReset, lambda: self.get_defaults().svg.abstract_storage)
			self._setup_reset_callback(self.ui.textureSVGMipmaps, self.ui.textureSVGMipmapsReset, lambda: self.get_defaults().svg.generate_mipmaps)
			self._setup_reset_callback(self.ui.textureSVGScale, self.ui.textureSVGScaleReset, lambda: self.get_defaults().svg.svg_scale)
			self._setup_reset_callback(self.ui.textureSVGMinFilter, self.ui.textureSVGMinFilterReset, lambda: self.get_defaults().svg.min_filter)
			self._setup_reset_callback(self.ui.textureSVGMagFilter, self.ui.textureSVGMagFilterReset, lambda: self.get_defaults().svg.mag_filter)
			self._setup_reset_callback(self.ui.textureSVGWrapS, self.ui.textureSVGWrapSReset, lambda: self.get_defaults().svg.wrap_s)
			self._setup_reset_callback(self.ui.textureSVGWrapT, self.ui.textureSVGWrapTReset, lambda: self.get_defaults().svg.wrap_t)

	@staticmethod
	def _update_reset_state(control: QWidget, default, reset_button: QPushButton):
		if SettingsForm.get_value(control) == default:
			reset_button.hide()
		else:
			reset_button.show()

	def update_reset_states(self):
		default = self.get_defaults()
		if self.is_raster:
			self._update_reset_state(self.ui.textureRasterStorage, default.raster.storage, self.ui.textureRasterStorageReset)
			self._update_reset_state(self.ui.textureRasterMipmaps, default.raster.generate_mipmaps, self.ui.textureRasterMipmapsReset)
			self._update_reset_state(self.ui.textureRasterMinFilter, default.raster.min_filter, self.ui.textureRasterMinFilterReset)
			self._update_reset_state(self.ui.textureRasterMagFilter, default.raster.mag_filter, self.ui.textureRasterMagFilterReset)
			self._update_reset_state(self.ui.textureRasterWrapS, default.raster.wrap_s, self.ui.textureRasterWrapSReset)
			self._update_reset_state(self.ui.textureRasterWrapT, default.raster.wrap_t, self.ui.textureRasterWrapTReset)
		else:
			self._update_reset_state(self.ui.textureSVGImageStorage, default.svg.image_storage, self.ui.textureSVGImageStorageReset)
			self._update_reset_state(self.ui.textureSVGAbstractStorage, default.svg.abstract_storage, self.ui.textureSVGAbstractStorageReset)
			self._update_reset_state(self.ui.textureSVGMipmaps, default.svg.generate_mipmaps, self.ui.textureSVGMipmapsReset)
			self._update_reset_state(self.ui.textureSVGScale, default.svg.svg_scale, self.ui.textureSVGScaleReset)
			self._update_reset_state(self.ui.textureSVGMinFilter, default.svg.min_filter, self.ui.textureSVGMinFilterReset)
			self._update_reset_state(self.ui.textureSVGMagFilter, default.svg.mag_filter, self.ui.textureSVGMagFilterReset)
			self._update_reset_state(self.ui.textureSVGWrapS, default.svg.wrap_s, self.ui.textureSVGWrapSReset)
			self._update_reset_state(self.ui.textureSVGWrapT, default.svg.wrap_t, self.ui.textureSVGWrapTReset)

	def spritesheet_toggled(self):
		for i in range(self.ui.spritesheetForm.count()):
			item = self.ui.spritesheetForm.itemAt(i)
			item.widget().show() if self.ui.createSpritesheet.isChecked() else item.widget().hide()

	def select_texture_slot(self):
		self.convert_to_ui_from_texture()

	def new_texture_slot(self):
		defaults = self.get_defaults()
		slot = TextureSlot(self.is_raster, self.is_gif)
		if self.is_raster:
			slot.raster = defaults.raster
		else:
			slot.svg = defaults.svg
		self.scratch_textures.append(slot)
		index = self.ui.textureSlotSelect.count()
		with block_signals(self.ui.textureSlotSelect) as textureSlotSelect:
			textureSlotSelect.addItem(f"{index}")
		self.ui.textureSlotSelect.setCurrentIndex(index)

	def delete_texture_slot(self):
		index = self.ui.textureSlotSelect.currentIndex()

		if len(self.scratch_textures) > 1:
			self.scratch_textures.pop(index)
			with block_signals(self.ui.textureSlotSelect) as textureSlotSelect:
				textureSlotSelect.removeItem(index)
				for i in range(index, textureSlotSelect.count()):
					textureSlotSelect.setItemText(i, f"{i}")
		else:
			assert index == 0
			defaults = self.get_defaults()
			slot = TextureSlot(self.is_raster, self.is_gif)
			if self.is_raster:
				slot.raster = defaults.raster
			else:
				slot.svg = defaults.svg
			self.scratch_textures[index] = slot

		self.select_texture_slot()

	def convert_to_texture_from_ui(self):
		Converters.convert_to_texture_from_ui(self.ui, self.scratch_textures[self.ui.textureSlotSelect.currentIndex()])
		if not self.is_raster:
			self.scratch_abstract_storage = PARAM_LIST.get_value(self.ui.textureSVGAbstractStorage.currentText())

	def convert_to_ui_from_texture(self):
		Converters.convert_to_ui_from_texture(self.ui, self.scratch_textures[self.ui.textureSlotSelect.currentIndex()])
		if not self.is_raster:
			with block_signals(self.ui.textureSVGAbstractStorage) as textureSVGAbstractStorage:
				textureSVGAbstractStorage.setCurrentText(PARAM_LIST.get_name(self.scratch_abstract_storage))
		if not self.is_gif:
			self.spritesheet_toggled()
		self.update_reset_states()
