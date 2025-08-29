from PySide6.QtWidgets import QLineEdit, QSpinBox

from ..asset_structures import FontFace, FontAtlas, Kerning

from editor.ui.tabs.Font import Ui_Font
from ... import PARAM_LIST, block_signals


def convert_to_font_from_ui(ui: Ui_Font, font_face: FontFace, font_atlas: FontAtlas):
	font_face.storage = PARAM_LIST.get_value(ui.fontFaceStorage.currentText())

	font_atlas.storage = PARAM_LIST.get_value(ui.fontAtlasStorage.currentText())
	font_atlas.font_size = ui.fontSize.value()
	font_atlas.min_filter = PARAM_LIST.get_value(ui.fontMinFilter.currentText())
	font_atlas.mag_filter = PARAM_LIST.get_value(ui.fontMagFilter.currentText())
	font_atlas.generate_mipmaps = ui.fontMipmaps.isChecked()
	font_atlas.common_buffer_preset = PARAM_LIST.get_value(ui.fontCommonBufferPreset.currentText())
	font_atlas.common_buffer = ui.fontCommonBuffer.text()
	font_atlas.use_common_buffer_preset = ui.fontUseBufferPreset.isChecked()


def convert_to_ui_from_font(ui: Ui_Font, font_face: FontFace, font_atlas: FontAtlas):
	ui.fontFaceStorage.setCurrentText(PARAM_LIST.get_name(font_face.storage))

	ui.fontAtlasStorage.setCurrentText(PARAM_LIST.get_name(font_atlas.storage))
	ui.fontSize.setValue(font_atlas.font_size)
	ui.fontMinFilter.setCurrentText(PARAM_LIST.get_name(font_atlas.min_filter))
	ui.fontMagFilter.setCurrentText(PARAM_LIST.get_name(font_atlas.mag_filter))
	ui.fontMipmaps.setChecked(font_atlas.generate_mipmaps)
	ui.fontCommonBufferPreset.setCurrentText(PARAM_LIST.get_name(font_atlas.common_buffer_preset))
	ui.fontCommonBuffer.setText(font_atlas.common_buffer)
	ui.fontUseBufferPreset.setChecked(font_atlas.use_common_buffer_preset)


def convert_to_kerning_from_ui(ui: Ui_Font, kerning_list: list[Kerning]):
	for row in range(ui.kerningTable.rowCount()):
		left_glyph = ui.kerningTable.cellWidget(row, 0)
		assert isinstance(left_glyph, QLineEdit)
		kerning_list[row].pair[0] = left_glyph.text()

		right_glyph = ui.kerningTable.cellWidget(row, 1)
		assert isinstance(right_glyph, QLineEdit)
		kerning_list[row].pair[1] = right_glyph.text()

		kerning = ui.kerningTable.cellWidget(row, 2)
		assert isinstance(kerning, QSpinBox)
		kerning_list[row].dist = kerning.value()


def convert_to_ui_from_kerning(ui: Ui_Font, kerning_list: list[Kerning]):
	for row in range(ui.kerningTable.rowCount()):
		left_glyph = ui.kerningTable.cellWidget(row, 0)
		assert isinstance(left_glyph, QLineEdit)
		with block_signals(left_glyph):
			left_glyph.setText(kerning_list[row].pair[0])

		right_glyph = ui.kerningTable.cellWidget(row, 1)
		assert isinstance(right_glyph, QLineEdit)
		with block_signals(right_glyph):
			right_glyph.setText(kerning_list[row].pair[1])

		kerning = ui.kerningTable.cellWidget(row, 2)
		assert isinstance(kerning, QSpinBox)
		with block_signals(kerning):
			kerning.setValue(kerning_list[row].dist)
