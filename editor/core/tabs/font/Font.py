from typing import override

from PySide6.QtCore import QSize
from PySide6.QtGui import QFontDatabase, QIcon
from PySide6.QtWidgets import QWidget, QPushButton, QHeaderView, QLineEdit, QSpinBox, QAbstractScrollArea

from editor import ui, TOMLAdapter
from editor.core import MainWindow, ImportedFontPathItem, block_signals
from . import Converters
from ..EditorTab import EditorTab
from ..asset_defaults.Defaults import Defaults
from ..asset_structures import FontFace, FontAtlas, Kerning
from ...common import SettingsForm
from ...common.SettingsForm import handle_all_children_modification


class FontTab(EditorTab):
	def __init__(self, win: MainWindow, item: ImportedFontPathItem):
		super().__init__(win)
		self.item = item

		self.scratch_font_face = FontFace()
		self.scratch_kerning_list: list[Kerning] = []
		self.scratch_font_atlases: list[FontAtlas] = []

		self.ui = ui.Font.Ui_Font()
		self.ui.setupUi(self)

		self.setup_preview()

		self.ui.kerningTable.horizontalHeader().setSectionResizeMode(0, QHeaderView.ResizeMode.Stretch)
		self.ui.kerningTable.horizontalHeader().setSectionResizeMode(1, QHeaderView.ResizeMode.Stretch)
		self.ui.kerningTable.horizontalHeader().setSectionResizeMode(2, QHeaderView.ResizeMode.Stretch)
		self.ui.kerningTable.horizontalHeader().setSectionResizeMode(3, QHeaderView.ResizeMode.Fixed)
		self.ui.kerningTable.setSizeAdjustPolicy(QAbstractScrollArea.SizeAdjustPolicy.AdjustToContents)

		self.ui.appendKerning.clicked.connect(self.append_kerning)
		self.ui.clearKerning.clicked.connect(self.clear_kerning)

		self.ui.newFontAtlas.clicked.connect(self.new_font_atlas)
		self.ui.deleteFontAtlas.clicked.connect(self.delete_font_atlas)
		self.ui.fontAtlasSelect.currentIndexChanged.connect(self.select_font_atlas)

		self.ui.fontUseBufferPreset.checkStateChanged.connect(self.toggle_common_buffer)
		with block_signals(self.ui.fontUseBufferPreset) as fontUseBufferPreset:
			fontUseBufferPreset.setChecked(True)
		self.toggle_common_buffer()

		self.setup_reset_callbacks()

		self.revert_changes_impl()

		handle_all_children_modification(self.ui.fontFaceSettings, self.control_modified)
		handle_all_children_modification(self.ui.slotSettings, self.control_modified)
		self.ui.newFontAtlas.clicked.connect(self.control_modified)
		self.ui.deleteFontAtlas.clicked.connect(self.control_modified)

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
		d: dict = {
			'font_face': self.scratch_font_face.to_dict(),
			'font_atlas': [a.to_dict() for a in self.scratch_font_atlases]
		}
		d['font_face']['kerning'] = [k.to_dict() for k in self.scratch_kerning_list]
		TOMLAdapter.dump(self.item.import_path, d, {'type': 'font'})

	@override
	def revert_changes_impl(self):
		d = TOMLAdapter.load(self.item.import_path)
		self.scratch_kerning_list = []
		if 'font_face' in d:
			font_face = d['font_face']
			self.scratch_font_face = FontFace.from_dict(font_face)
			if 'kerning' in font_face:
				self.scratch_kerning_list = [Kerning.from_dict(k) for k in font_face['kerning']]
		else:
			self.scratch_font_face = FontFace()
		if 'font_atlas' in d and len(d['font_atlas']) > 0:
			self.scratch_font_atlases = [FontAtlas.from_dict(a) for a in d['font_atlas']]
		else:
			self.scratch_font_atlases = [FontAtlas()]

		with block_signals(self.ui.kerningTable) as kerningTable:
			for _ in range(kerningTable.rowCount()):
				kerningTable.removeRow(0)
			for _ in range(len(self.scratch_kerning_list)):
				self.add_kerning_row_to_ui()
			self.convert_to_ui_from_kerning()

		with block_signals(self.ui.fontAtlasSelect) as fontAtlasSelect:
			fontAtlasSelect.clear()
			for i in range(len(self.scratch_font_atlases)):
				fontAtlasSelect.addItem(f"{i}")
			fontAtlasSelect.setCurrentIndex(0)
			self.select_font_atlas()

	@override
	def rename_impl(self, item: ImportedFontPathItem):
		assert isinstance(item, ImportedFontPathItem)
		self.item = item

	@override
	def refresh_impl(self):
		self.setup_preview()
		self.update_reset_states()

	def setup_preview(self):
		font_id = QFontDatabase.addApplicationFont(self.item.full_path.as_posix())
		assert font_id >= 0
		font_family = QFontDatabase.applicationFontFamilies(font_id)[0]
		font = self.ui.previewFont.font()
		font.setFamily(font_family)
		self.ui.previewFont.setFont(font)

	def control_modified(self):
		self.set_asterisk(True)
		self.convert_to_font_from_ui()
		self.update_reset_states()

	def get_defaults(self):
		return Defaults.from_dict(TOMLAdapter.load(self.win.project_context.asset_defaults_file)).font

	def toggle_common_buffer(self):
		if self.ui.fontUseBufferPreset.isChecked():
			self.ui.fontCommonBufferPreset.setEnabled(True)
			self.ui.fontCommonBufferPresetReset.setEnabled(True)
			self.ui.fontCommonBuffer.setDisabled(True)
			self.ui.fontCommonBufferReset.setDisabled(True)
		else:
			self.ui.fontCommonBufferPreset.setDisabled(True)
			self.ui.fontCommonBufferPresetReset.setDisabled(True)
			self.ui.fontCommonBuffer.setEnabled(True)
			self.ui.fontCommonBufferReset.setEnabled(True)

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
		self._setup_reset_callback(self.ui.fontFaceStorage, self.ui.fontFaceStorageReset, lambda: self.get_defaults().font_face.storage)
		self._setup_reset_callback(self.ui.fontAtlasStorage, self.ui.fontAtlasStorageReset, lambda: self.get_defaults().font_atlas.storage)
		self._setup_reset_callback(self.ui.fontSize, self.ui.fontSizeReset, lambda: self.get_defaults().font_atlas.font_size)
		self._setup_reset_callback(self.ui.fontMinFilter, self.ui.fontMinFilterReset, lambda: self.get_defaults().font_atlas.min_filter)
		self._setup_reset_callback(self.ui.fontMagFilter, self.ui.fontMagFilterReset, lambda: self.get_defaults().font_atlas.mag_filter)
		self._setup_reset_callback(self.ui.fontMipmaps, self.ui.fontMipmapsReset, lambda: self.get_defaults().font_atlas.generate_mipmaps)
		self._setup_reset_callback(self.ui.fontCommonBufferPreset, self.ui.fontCommonBufferPresetReset, lambda: self.get_defaults().font_atlas.common_buffer_preset)
		self._setup_reset_callback(self.ui.fontCommonBuffer, self.ui.fontCommonBufferReset, lambda: self.get_defaults().font_atlas.common_buffer)
		self._setup_reset_callback(self.ui.fontUseBufferPreset, self.ui.fontUseBufferPresetReset, lambda: self.get_defaults().font_atlas.use_common_buffer_preset)

	@staticmethod
	def _update_reset_state(control: QWidget, default, reset_button: QPushButton):
		if SettingsForm.get_value(control) == default:
			reset_button.hide()
		else:
			reset_button.show()

	def update_reset_states(self):
		defaults = self.get_defaults()
		self._update_reset_state(self.ui.fontFaceStorage, defaults.font_face.storage, self.ui.fontFaceStorageReset)
		self._update_reset_state(self.ui.fontAtlasStorage, defaults.font_atlas.storage, self.ui.fontAtlasStorageReset)
		self._update_reset_state(self.ui.fontSize, defaults.font_atlas.font_size, self.ui.fontSizeReset)
		self._update_reset_state(self.ui.fontMinFilter, defaults.font_atlas.min_filter, self.ui.fontMinFilterReset)
		self._update_reset_state(self.ui.fontMagFilter, defaults.font_atlas.mag_filter, self.ui.fontMagFilterReset)
		self._update_reset_state(self.ui.fontMipmaps, defaults.font_atlas.generate_mipmaps, self.ui.fontMipmapsReset)
		self._update_reset_state(self.ui.fontCommonBufferPreset, defaults.font_atlas.common_buffer_preset, self.ui.fontCommonBufferPresetReset)
		self._update_reset_state(self.ui.fontCommonBuffer, defaults.font_atlas.common_buffer, self.ui.fontCommonBufferReset)
		self._update_reset_state(self.ui.fontUseBufferPreset, defaults.font_atlas.use_common_buffer_preset, self.ui.fontUseBufferPresetReset)

	def glyph_callback(self, col, row):
		def callback():
			self.set_asterisk(True)
			widget = self.ui.kerningTable.cellWidget(row, col)
			assert isinstance(widget, QLineEdit)
			self.scratch_kerning_list[row].pair[col] = widget.text()
		return callback

	def kerning_callback(self, row):
		def callback():
			self.set_asterisk(True)
			widget = self.ui.kerningTable.cellWidget(row, 2)
			assert isinstance(widget, QSpinBox)
			self.scratch_kerning_list[row].dist = widget.value()
		return callback

	def delete_kerning_callback(self, row):
		def callback():
			self.scratch_kerning_list.pop(row)
			self.ui.kerningTable.removeRow(row)
			for i in range(row, self.ui.kerningTable.rowCount()):
				left_glyph = self.ui.kerningTable.cellWidget(i, 0)
				assert isinstance(left_glyph, QLineEdit)
				left_glyph.textChanged.disconnect()
				left_glyph.textChanged.connect(self.glyph_callback(0, i))

				right_glyph = self.ui.kerningTable.cellWidget(i, 1)
				assert isinstance(right_glyph, QLineEdit)
				right_glyph.textChanged.disconnect()
				right_glyph.textChanged.connect(self.glyph_callback(1, i))

				kerning = self.ui.kerningTable.cellWidget(i, 2)
				assert isinstance(kerning, QSpinBox)
				kerning.valueChanged.disconnect()
				kerning.valueChanged.connect(self.kerning_callback(i))

				delete_row = self.ui.kerningTable.cellWidget(i, 3)
				assert isinstance(delete_row, QPushButton)
				delete_row.clicked.disconnect()
				delete_row.clicked.connect(self.delete_kerning_callback(i))
		return callback

	def append_kerning(self):
		self.scratch_kerning_list.append(Kerning())
		self.add_kerning_row_to_ui()

	def add_kerning_row_to_ui(self):
		row = self.ui.kerningTable.rowCount()
		left_glyph = QLineEdit()
		left_glyph.textChanged.connect(self.glyph_callback(0, row))
		right_glyph = QLineEdit()
		right_glyph.textChanged.connect(self.glyph_callback(1, row))
		kerning = QSpinBox(minimum=-2048, maximum=2048)
		kerning.valueChanged.connect(self.kerning_callback(row))
		delete_row = QPushButton(QIcon("res/images/Delete.png"), "")
		delete_row.clicked.connect(self.delete_kerning_callback(row))
		self.ui.kerningTable.insertRow(row)
		self.ui.kerningTable.setCellWidget(row, 0, left_glyph)
		self.ui.kerningTable.setCellWidget(row, 1, right_glyph)
		self.ui.kerningTable.setCellWidget(row, 2, kerning)
		self.ui.kerningTable.setCellWidget(row, 3, delete_row)

	def clear_kerning(self):
		self.scratch_kerning_list.clear()
		for _ in range(self.ui.kerningTable.rowCount()):
			self.ui.kerningTable.removeRow(0)

	def new_font_atlas(self):
		font_atlas = FontAtlas()
		self.scratch_font_atlases.append(font_atlas)
		with block_signals(self.ui.fontAtlasSelect) as fontAtlasSelect:
			slot = fontAtlasSelect.count()
			fontAtlasSelect.addItem(f"{slot}")
			fontAtlasSelect.setCurrentIndex(slot)
		self.select_font_atlas()

	def delete_font_atlas(self):
		slot_count = self.ui.fontAtlasSelect.count()
		slot = self.ui.fontAtlasSelect.currentIndex()
		if slot_count > 1:
			self.scratch_font_atlases.pop(slot)
			with block_signals(self.ui.fontAtlasSelect) as fontAtlasSelect:
				fontAtlasSelect.removeItem(slot)
				for i in range(slot, fontAtlasSelect.count()):
					fontAtlasSelect.setItemText(i, f"{i}")
			self.select_font_atlas()
		else:
			assert slot == 0
			self.scratch_font_atlases[slot] = FontAtlas()
		self.select_font_atlas()

	def select_font_atlas(self):
		self.convert_to_ui_from_font()

	def convert_to_font_from_ui(self):
		Converters.convert_to_font_from_ui(self.ui, self.scratch_font_face, self.scratch_font_atlases[self.ui.fontAtlasSelect.currentIndex()])

	def convert_to_ui_from_font(self):
		Converters.convert_to_ui_from_font(self.ui, self.scratch_font_face, self.scratch_font_atlases[self.ui.fontAtlasSelect.currentIndex()])

	def convert_to_kerning_from_ui(self):
		Converters.convert_to_kerning_from_ui(self.ui, self.scratch_kerning_list)

	def convert_to_ui_from_kerning(self):
		Converters.convert_to_ui_from_kerning(self.ui, self.scratch_kerning_list)
