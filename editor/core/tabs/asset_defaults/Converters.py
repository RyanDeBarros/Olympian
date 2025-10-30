from editor.ui.tabs.AssetDefaults import Ui_AssetDefaults
from editor.core import PARAM_LIST
from ..asset_structures import Texture, Font


def convert_to_texture_from_ui(ui: Ui_AssetDefaults, texture: Texture):
	texture.raster.storage = PARAM_LIST.get_value(ui.textureRasterStorage.currentText())
	texture.raster.generate_mipmaps = ui.textureRasterMipmaps.isChecked()
	texture.raster.min_filter = PARAM_LIST.get_value(ui.textureRasterMinFilter.currentText())
	texture.raster.mag_filter = PARAM_LIST.get_value(ui.textureRasterMagFilter.currentText())
	texture.raster.wrap_s = PARAM_LIST.get_value(ui.textureRasterWrapS.currentText())
	texture.raster.wrap_t = PARAM_LIST.get_value(ui.textureRasterWrapT.currentText())

	texture.svg.image_storage = PARAM_LIST.get_value(ui.textureSVGImageStorage.currentText())
	texture.svg.abstract_storage = PARAM_LIST.get_value(ui.textureSVGAbstractStorage.currentText())
	texture.svg.generate_mipmaps = PARAM_LIST.get_value(ui.textureSVGMipmaps.currentText())
	texture.svg.svg_scale = ui.textureSVGScale.value()
	texture.svg.min_filter = PARAM_LIST.get_value(ui.textureSVGMinFilter.currentText())
	texture.svg.mag_filter = PARAM_LIST.get_value(ui.textureSVGMagFilter.currentText())
	texture.svg.wrap_s = PARAM_LIST.get_value(ui.textureSVGWrapS.currentText())
	texture.svg.wrap_t = PARAM_LIST.get_value(ui.textureSVGWrapT.currentText())


def convert_to_ui_from_texture(ui: Ui_AssetDefaults, texture: Texture):
	ui.textureRasterStorage.setCurrentText(PARAM_LIST.get_name(texture.raster.storage))
	ui.textureRasterMipmaps.setChecked(texture.raster.generate_mipmaps)
	ui.textureRasterMinFilter.setCurrentText(PARAM_LIST.get_name(texture.raster.min_filter))
	ui.textureRasterMagFilter.setCurrentText(PARAM_LIST.get_name(texture.raster.mag_filter))
	ui.textureRasterWrapS.setCurrentText(PARAM_LIST.get_name(texture.raster.wrap_s))
	ui.textureRasterWrapT.setCurrentText(PARAM_LIST.get_name(texture.raster.wrap_t))

	ui.textureSVGImageStorage.setCurrentText(PARAM_LIST.get_name(texture.svg.image_storage))
	ui.textureSVGAbstractStorage.setCurrentText(PARAM_LIST.get_name(texture.svg.abstract_storage))
	ui.textureSVGMipmaps.setCurrentText(PARAM_LIST.get_name(texture.svg.generate_mipmaps))
	ui.textureSVGScale.setValue(texture.svg.svg_scale)
	ui.textureSVGMinFilter.setCurrentText(PARAM_LIST.get_name(texture.svg.min_filter))
	ui.textureSVGMagFilter.setCurrentText(PARAM_LIST.get_name(texture.svg.mag_filter))
	ui.textureSVGWrapS.setCurrentText(PARAM_LIST.get_name(texture.svg.wrap_s))
	ui.textureSVGWrapT.setCurrentText(PARAM_LIST.get_name(texture.svg.wrap_t))


def convert_to_font_from_ui(ui: Ui_AssetDefaults, font: Font):
	font.font_face.storage = PARAM_LIST.get_value(ui.fontFaceStorage.currentText())

	font.font_atlas.storage = PARAM_LIST.get_value(ui.fontAtlasStorage.currentText())
	font.font_atlas.font_size = ui.fontSize.value()
	font.font_atlas.min_filter = PARAM_LIST.get_value(ui.fontMinFilter.currentText())
	font.font_atlas.mag_filter = PARAM_LIST.get_value(ui.fontMagFilter.currentText())
	font.font_atlas.generate_mipmaps = ui.fontMipmaps.isChecked()
	font.font_atlas.common_buffer_preset = PARAM_LIST.get_value(ui.fontCommonBufferPreset.currentText())
	font.font_atlas.common_buffer = ui.fontCommonBuffer.text()

	font.font_atlas.use_common_buffer_preset = ui.fontUseBufferPreset.isChecked()


def convert_to_ui_from_font(ui: Ui_AssetDefaults, font: Font):
	ui.fontFaceStorage.setCurrentText(PARAM_LIST.get_name(font.font_face.storage))

	ui.fontAtlasStorage.setCurrentText(PARAM_LIST.get_name(font.font_atlas.storage))
	ui.fontSize.setValue(font.font_atlas.font_size)
	ui.fontMinFilter.setCurrentText(PARAM_LIST.get_name(font.font_atlas.min_filter))
	ui.fontMagFilter.setCurrentText(PARAM_LIST.get_name(font.font_atlas.mag_filter))
	ui.fontMipmaps.setChecked(font.font_atlas.generate_mipmaps)
	ui.fontCommonBufferPreset.setCurrentText(PARAM_LIST.get_name(font.font_atlas.common_buffer_preset))
	ui.fontCommonBuffer.setText(font.font_atlas.common_buffer)

	ui.fontUseBufferPreset.setChecked(font.font_atlas.use_common_buffer_preset)
