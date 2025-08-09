import os
import posixpath
from pathlib import Path

import toml
from PySide6.QtWidgets import QWidget, QFileDialog

from editor import ui, MANIFEST, ProjectContext
from editor.Params import *


class TextureEditorWidget(QWidget):
	def __init__(self, win):
		super().__init__()
		self.win = win

		self.ui = ui.asset_editors.Texture.Ui_Form()
		self.ui.setupUi(self)

		self.edit_tab = EditTab(self)
		self.defaults_tab = DefaultsTab(self)
		self.import_tab = ImportTab(self)

		self.last_file_dialog_dir = posixpath.join(posixpath.dirname(ProjectContext.PROJECT_FILE), "res")


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

		self.validate_project_filepaths()

		self.load_defaults()

	def validate_project_filepaths(self):
		filepath = self.default_raster_texture_filepath()
		os.makedirs(os.path.dirname(filepath), exist_ok=True)
		if not os.path.exists(filepath):
			self.save_raster_defaults()

		filepath = self.default_svg_texture_filepath()
		os.makedirs(os.path.dirname(filepath), exist_ok=True)
		if not os.path.exists(filepath):
			self.save_svg_defaults()

	def default_raster_texture_filepath(self):
		project_id = MANIFEST.get_project_id(ProjectContext.PROJECT_FILE)
		return f'projects/{project_id}/asset_defaults/raster_texture.toml'

	def default_svg_texture_filepath(self):
		project_id = MANIFEST.get_project_id(ProjectContext.PROJECT_FILE)
		return f'projects/{project_id}/asset_defaults/svg_texture.toml'

	def get_stored_default_raster_dict(self):
		with open(self.default_raster_texture_filepath(), 'r') as f:
			return toml.load(f)

	def _get_default_raster_dict(self):
		return {
			'storage': PARAM_LIST.get_value(self.ui.defaultImageStorage.currentText()),
			'min filter': PARAM_LIST.get_value(self.ui.defaultImageMinFilter.currentText()),
			'mag filter': PARAM_LIST.get_value(self.ui.defaultImageMagFilter.currentText()),
			'generate mipmaps': self.ui.defaultImageMipmaps.isChecked(),
			'wrap s': PARAM_LIST.get_value(self.ui.defaultImageWrapS.currentText()),
			'wrap t': PARAM_LIST.get_value(self.ui.defaultImageWrapT.currentText())
		}

	def get_stored_default_svg_dict(self):
		with open(self.default_svg_texture_filepath(), 'r') as f:
			return toml.load(f)

	def _get_default_svg_dict(self):
		return {
			'abstract storage': PARAM_LIST.get_value(self.ui.defaultSVGAbstractStorage.currentText()),
			'image storage': PARAM_LIST.get_value(self.ui.defaultSVGImageStorage.currentText()),
			'min filter': PARAM_LIST.get_value(self.ui.defaultSVGMinFilter.currentText()),
			'mag filter': PARAM_LIST.get_value(self.ui.defaultSVGMagFilter.currentText()),
			'generate mipmaps': PARAM_LIST.get_value(self.ui.defaultSVGMipmaps.currentText()),
			'wrap s': PARAM_LIST.get_value(self.ui.defaultSVGWrapS.currentText()),
			'wrap t': PARAM_LIST.get_value(self.ui.defaultSVGWrapT.currentText())
		}

	def load_defaults(self):
		defaults = self.get_stored_default_raster_dict()
		self.ui.defaultImageStorage.setCurrentText(PARAM_LIST.get_name(defaults['storage']))
		self.ui.defaultImageMinFilter.setCurrentText(PARAM_LIST.get_name(defaults['min filter']))
		self.ui.defaultImageMagFilter.setCurrentText(PARAM_LIST.get_name(defaults['mag filter']))
		self.ui.defaultImageMipmaps.setChecked(defaults['generate mipmaps'])
		self.ui.defaultImageWrapS.setCurrentText(PARAM_LIST.get_name(defaults['wrap s']))
		self.ui.defaultImageWrapT.setCurrentText(PARAM_LIST.get_name(defaults['wrap t']))

		defaults = self.get_stored_default_svg_dict()
		self.ui.defaultSVGAbstractStorage.setCurrentText(PARAM_LIST.get_name(defaults['abstract storage']))
		self.ui.defaultSVGImageStorage.setCurrentText(PARAM_LIST.get_name(defaults['image storage']))
		self.ui.defaultSVGMinFilter.setCurrentText(PARAM_LIST.get_name(defaults['min filter']))
		self.ui.defaultSVGMagFilter.setCurrentText(PARAM_LIST.get_name(defaults['mag filter']))
		self.ui.defaultSVGMipmaps.setCurrentText(PARAM_LIST.get_name(defaults['generate mipmaps']))
		self.ui.defaultSVGWrapS.setCurrentText(PARAM_LIST.get_name(defaults['wrap s']))
		self.ui.defaultSVGWrapT.setCurrentText(PARAM_LIST.get_name(defaults['wrap t']))

	def save_defaults(self):
		self.save_raster_defaults()
		self.save_svg_defaults()

	def save_raster_defaults(self):
		with open(self.default_raster_texture_filepath(), 'w') as f:
			toml.dump(self._get_default_raster_dict(), f)

	def save_svg_defaults(self):
		with open(self.default_svg_texture_filepath(), 'w') as f:
			toml.dump(self._get_default_svg_dict(), f)


class ImportTab:
	def __init__(self, texture_editor: TextureEditorWidget):
		self.editor = texture_editor
