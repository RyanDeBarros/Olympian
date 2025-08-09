import os
import posixpath
from pathlib import Path

import toml
from PySide6.QtWidgets import QWidget, QFileDialog

from editor import ui
from editor.GLParams import *


class TextureEditorWidget(QWidget):
	def __init__(self, win):
		super().__init__()
		self.win = win

		self.ui = ui.asset_editors.Texture.Ui_Form()
		self.ui.setupUi(self)

		self.edit_tab = EditTab(self)
		self.defaults_tab = DefaultsTab(self)
		self.import_tab = ImportTab(self)

		self.last_file_dialog_dir = posixpath.join(posixpath.dirname(win.project_filepath), "res")


class EditTab:
	def __init__(self, texture_editor: TextureEditorWidget):
		self.editor = texture_editor
		self.ui = self.editor.ui
		self.texture = []

		self.ui.editTextureBrowse.clicked.connect(self.browse_texture)
		self.ui.editTextureFilepath.textChanged.connect(self.texture_filepath_changed)
		self.ui.editSpritesheet.checkStateChanged.connect(self.spritesheet_checked_changed)

		self.texture_filepath_changed()

	def browse_texture(self):
		filepath, _ = QFileDialog.getOpenFileName(self.editor, "Select File", self.editor.last_file_dialog_dir,
												  filter="Image files (*.png *.gif *.svg *.jpg *.jpeg *.bmp *.tga)")
		if filepath:
			self.editor.last_file_dialog_dir = os.path.dirname(filepath)
			self.ui.editTextureFilepath.setText(filepath)

	def texture_filepath_changed(self):
		filepath = self.ui.editTextureFilepath.text()
		if len(filepath) > 0:
			self.ui.paramsLayout.show()
			self.ui.editTextureSlotCombo.setCurrentIndex(0)

			if Path(filepath).suffix != ".svg":
				self.ui.editParams.setCurrentWidget(self.ui.editImageParams)
			else:
				self.ui.editParams.setCurrentWidget(self.ui.editSVGParams)

			if Path(filepath).suffix != ".gif":
				self.ui.editSpritesheetWidget.show()

				# TODO v3 check if spritesheet exists at slot 0
				self.ui.editSpritesheet.setChecked(False)
				self.spritesheet_checked_changed()
			else:
				self.ui.editSpritesheetWidget.hide()

			# TODO v3 load or create self.texture
		else:
			self.ui.paramsLayout.hide()

	def spritesheet_checked_changed(self):
		if self.ui.editSpritesheet.isChecked():
			# TODO v3 load spritesheet params if exist in import
			self.ui.editSpritesheetParams.show()
		else:
			self.ui.editSpritesheetParams.hide()


class DefaultsTab:
	def __init__(self, texture_editor: TextureEditorWidget):
		self.editor = texture_editor
		self.ui = self.editor.ui
		self.ui.saveDefaultsButton.clicked.connect(self.save_defaults)

		self.load_defaults()

	def get_stored_default_raster_dict(self):
		with open('data/asset_defaults/raster_texture.toml', 'r') as f:
			return toml.load(f)

	def _get_default_raster_dict(self):
		return {
			'storage': self.ui.defaultImageStorage.currentText().lower(),
			'min filter': GL_PARAM_LIST.get_value(self.ui.defaultImageMinFilter.currentText()),
			'mag filter': GL_PARAM_LIST.get_value(self.ui.defaultImageMagFilter.currentText()),
			'generate mipmaps': self.ui.defaultImageMipmaps.isChecked(),
			'wrap s': GL_PARAM_LIST.get_value(self.ui.defaultImageWrapS.currentText()),
			'wrap t': GL_PARAM_LIST.get_value(self.ui.defaultImageWrapT.currentText())
		}

	def get_stored_default_svg_dict(self):
		with open('data/asset_defaults/svg_texture.toml', 'r') as f:
			return toml.load(f)

	def _get_default_svg_dict(self):
		return {
			'abstract storage': self.ui.defaultSVGAbstractStorage.currentText().lower(),
			'image storage': self.ui.defaultSVGImageStorage.currentText().lower(),
			'min filter': GL_PARAM_LIST.get_value(self.ui.defaultSVGMinFilter.currentText()),
			'mag filter': GL_PARAM_LIST.get_value(self.ui.defaultSVGMagFilter.currentText()),
			'generate mipmaps': self.ui.defaultSVGMipmaps.currentText().lower(),
			'wrap s': GL_PARAM_LIST.get_value(self.ui.defaultSVGWrapS.currentText()),
			'wrap t': GL_PARAM_LIST.get_value(self.ui.defaultSVGWrapT.currentText())
		}

	def load_defaults(self):
		defaults = self.get_stored_default_raster_dict()
		self.ui.defaultImageStorage.setCurrentText(defaults['storage'].title())
		self.ui.defaultImageMinFilter.setCurrentText(GL_PARAM_LIST.get_name(defaults['min filter']))
		self.ui.defaultImageMagFilter.setCurrentText(GL_PARAM_LIST.get_name(defaults['mag filter']))
		self.ui.defaultImageMipmaps.setChecked(defaults['generate mipmaps'])
		self.ui.defaultImageWrapS.setCurrentText(GL_PARAM_LIST.get_name(defaults['wrap s']))
		self.ui.defaultImageWrapT.setCurrentText(GL_PARAM_LIST.get_name(defaults['wrap t']))

		defaults = self.get_stored_default_svg_dict()
		self.ui.defaultSVGAbstractStorage.setCurrentText(defaults['abstract storage'].title())
		self.ui.defaultSVGImageStorage.setCurrentText(defaults['image storage'].title())
		self.ui.defaultSVGMinFilter.setCurrentText(GL_PARAM_LIST.get_name(defaults['min filter']))
		self.ui.defaultSVGMagFilter.setCurrentText(GL_PARAM_LIST.get_name(defaults['mag filter']))
		self.ui.defaultSVGMipmaps.setCurrentText(defaults['generate mipmaps'].title())
		self.ui.defaultSVGWrapS.setCurrentText(GL_PARAM_LIST.get_name(defaults['wrap s']))
		self.ui.defaultSVGWrapT.setCurrentText(GL_PARAM_LIST.get_name(defaults['wrap t']))

	def save_defaults(self):
		with open('data/asset_defaults/raster_texture.toml', 'w') as f:
			toml.dump(self._get_default_raster_dict(), f)

		with open('data/asset_defaults/svg_texture.toml', 'w') as f:
			toml.dump(self._get_default_svg_dict(), f)


class ImportTab:
	def __init__(self, texture_editor: TextureEditorWidget):
		self.editor = texture_editor
