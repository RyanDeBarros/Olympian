from typing import Optional

from editor.ui.tabs.Texture import Ui_Texture
from editor.core import PARAM_LIST, block_all_signals
from editor.core.tabs.asset_structures import RasterTexture, SVGTexture, Spritesheet


class TextureSlot:
	def __init__(self, is_raster: bool, is_gif: bool):
		self.is_raster = is_raster
		self.is_gif = is_gif
		self.raster: Optional[RasterTexture] = RasterTexture() if self.is_raster else None
		self.svg: Optional[SVGTexture] = SVGTexture() if not self.is_raster else None
		self.spritesheet: Optional[Spritesheet] = Spritesheet() if not self.is_gif else None

	def get_dict(self):
		d = {}
		if self.is_raster:
			d.update(self.raster.to_dict())
		else:
			d.update(self.svg.to_dict())
			del d['abstract_storage']
		if not self.is_gif:
			if self.spritesheet.anim:
				d.update(self.spritesheet.to_dict())
		return d

	def load_dict(self, d):
		if self.is_raster:
			self.raster = RasterTexture.from_dict(d)
		else:
			self.svg = SVGTexture.from_dict(d)
		if not self.is_gif:
			self.spritesheet = Spritesheet.from_dict(d)


def convert_to_texture_from_ui(ui: Ui_Texture, texture: TextureSlot):
	if texture.is_raster:
		texture.raster.storage = PARAM_LIST.get_value(ui.textureRasterStorage.currentText())
		texture.raster.generate_mipmaps = ui.textureRasterMipmaps.isChecked()
		texture.raster.min_filter = PARAM_LIST.get_value(ui.textureRasterMinFilter.currentText())
		texture.raster.mag_filter = PARAM_LIST.get_value(ui.textureRasterMagFilter.currentText())
		texture.raster.wrap_s = PARAM_LIST.get_value(ui.textureRasterWrapS.currentText())
		texture.raster.wrap_t = PARAM_LIST.get_value(ui.textureRasterWrapT.currentText())
	else:
		texture.svg.image_storage = PARAM_LIST.get_value(ui.textureSVGImageStorage.currentText())
		texture.svg.generate_mipmaps = PARAM_LIST.get_value(ui.textureSVGMipmaps.currentText())
		texture.svg.svg_scale = ui.textureSVGScale.value()
		texture.svg.min_filter = PARAM_LIST.get_value(ui.textureSVGMinFilter.currentText())
		texture.svg.mag_filter = PARAM_LIST.get_value(ui.textureSVGMagFilter.currentText())
		texture.svg.wrap_s = PARAM_LIST.get_value(ui.textureSVGWrapS.currentText())
		texture.svg.wrap_t = PARAM_LIST.get_value(ui.textureSVGWrapT.currentText())
	if not texture.is_gif:
		texture.spritesheet.anim = ui.createSpritesheet.isChecked()
		texture.spritesheet.rows = ui.spritesheetRows.value()
		texture.spritesheet.cols = ui.spritesheetCols.value()
		texture.spritesheet.cell_width_override = ui.spritesheetCellWidth.value()
		texture.spritesheet.cell_height_override = ui.spritesheetCellHeight.value()
		texture.spritesheet.delay_cs = ui.spritesheetDelayCS.value()
		texture.spritesheet.row_major = ui.spritesheetRowMajor.isChecked()
		texture.spritesheet.row_up = ui.spritesheetRowUp.isChecked()


def convert_to_ui_from_texture(ui: Ui_Texture, texture: TextureSlot):
	if texture.is_raster:
		with block_all_signals(ui.rasterSettings):
			ui.textureRasterStorage.setCurrentText(PARAM_LIST.get_name(texture.raster.storage))
			ui.textureRasterMipmaps.setChecked(texture.raster.generate_mipmaps)
			ui.textureRasterMinFilter.setCurrentText(PARAM_LIST.get_name(texture.raster.min_filter))
			ui.textureRasterMagFilter.setCurrentText(PARAM_LIST.get_name(texture.raster.mag_filter))
			ui.textureRasterWrapS.setCurrentText(PARAM_LIST.get_name(texture.raster.wrap_s))
			ui.textureRasterWrapT.setCurrentText(PARAM_LIST.get_name(texture.raster.wrap_t))
	else:
		with block_all_signals(ui.svgSettings):
			ui.textureSVGImageStorage.setCurrentText(PARAM_LIST.get_name(texture.svg.image_storage))
			ui.textureSVGMipmaps.setCurrentText(PARAM_LIST.get_name(texture.svg.generate_mipmaps))
			ui.textureSVGScale.setValue(texture.svg.svg_scale)
			ui.textureSVGMinFilter.setCurrentText(PARAM_LIST.get_name(texture.svg.min_filter))
			ui.textureSVGMagFilter.setCurrentText(PARAM_LIST.get_name(texture.svg.mag_filter))
			ui.textureSVGWrapS.setCurrentText(PARAM_LIST.get_name(texture.svg.wrap_s))
			ui.textureSVGWrapT.setCurrentText(PARAM_LIST.get_name(texture.svg.wrap_t))
	if not texture.is_gif:
		with block_all_signals(ui.spritesheetSettings):
			ui.createSpritesheet.setChecked(texture.spritesheet.anim)
			ui.spritesheetRows.setValue(texture.spritesheet.rows)
			ui.spritesheetCols.setValue(texture.spritesheet.cols)
			ui.spritesheetCellWidth.setValue(texture.spritesheet.cell_width_override)
			ui.spritesheetCellHeight.setValue(texture.spritesheet.cell_height_override)
			ui.spritesheetDelayCS.setValue(texture.spritesheet.delay_cs)
			ui.spritesheetRowMajor.setChecked(texture.spritesheet.row_major)
			ui.spritesheetRowUp.setChecked(texture.spritesheet.row_up)
