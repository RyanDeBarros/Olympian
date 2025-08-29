from editor import ui
from editor.core import PARAM_LIST, block_all_signals
from editor.core.tabs.asset_structures import RasterTexture, SVGTexture


def convert_to_raster_texture_from_ui(ui: ui.Texture.Ui_Texture, texture: RasterTexture):
	texture.storage = PARAM_LIST.get_value(ui.textureRasterStorage.currentText())
	texture.generate_mipmaps = ui.textureRasterMipmaps.isChecked()
	texture.min_filter = PARAM_LIST.get_value(ui.textureRasterMinFilter.currentText())
	texture.mag_filter = PARAM_LIST.get_value(ui.textureRasterMagFilter.currentText())
	texture.wrap_s = PARAM_LIST.get_value(ui.textureRasterWrapS.currentText())
	texture.wrap_t = PARAM_LIST.get_value(ui.textureRasterWrapT.currentText())


def convert_to_ui_from_raster_texture(ui: ui.Texture.Ui_Texture, texture: RasterTexture):
	with block_all_signals(ui.rasterSettings):
		ui.textureRasterStorage.setCurrentText(PARAM_LIST.get_name(texture.storage))
		ui.textureRasterMipmaps.setChecked(texture.generate_mipmaps)
		ui.textureRasterMinFilter.setCurrentText(PARAM_LIST.get_name(texture.min_filter))
		ui.textureRasterMagFilter.setCurrentText(PARAM_LIST.get_name(texture.mag_filter))
		ui.textureRasterWrapS.setCurrentText(PARAM_LIST.get_name(texture.wrap_s))
		ui.textureRasterWrapT.setCurrentText(PARAM_LIST.get_name(texture.wrap_t))


def convert_to_svg_texture_from_ui(ui: ui.Texture.Ui_Texture, texture: SVGTexture):
	texture.image_storage = PARAM_LIST.get_value(ui.textureSVGImageStorage.currentText())
	texture.generate_mipmaps = PARAM_LIST.get_value(ui.textureSVGMipmaps.currentText())
	texture.min_filter = PARAM_LIST.get_value(ui.textureSVGMinFilter.currentText())
	texture.mag_filter = PARAM_LIST.get_value(ui.textureSVGMagFilter.currentText())
	texture.wrap_s = PARAM_LIST.get_value(ui.textureSVGWrapS.currentText())
	texture.wrap_t = PARAM_LIST.get_value(ui.textureSVGWrapT.currentText())


def convert_to_ui_from_svg_texture(ui: ui.Texture.Ui_Texture, texture: SVGTexture):
	with block_all_signals(ui.svgSettings):
		ui.textureSVGImageStorage.setCurrentText(PARAM_LIST.get_name(texture.image_storage))
		ui.textureSVGMipmaps.setCurrentText(PARAM_LIST.get_name(texture.generate_mipmaps))
		ui.textureSVGMinFilter.setCurrentText(PARAM_LIST.get_name(texture.min_filter))
		ui.textureSVGMagFilter.setCurrentText(PARAM_LIST.get_name(texture.mag_filter))
		ui.textureSVGWrapS.setCurrentText(PARAM_LIST.get_name(texture.wrap_s))
		ui.textureSVGWrapT.setCurrentText(PARAM_LIST.get_name(texture.wrap_t))
