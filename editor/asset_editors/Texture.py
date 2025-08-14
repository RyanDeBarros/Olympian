import os
from pathlib import Path
from typing import List, Optional

import toml
from PySide6.QtWidgets import QWidget, QFileDialog, QMessageBox

from editor import ui, MANIFEST
from editor.util import *
from .Common import SettingsForm, SettingsParameter


class TextureEditorWidget(QWidget):
	def __init__(self, win):
		super().__init__()
		self.win = win

		self.ui = ui.asset_editors.Texture.Ui_Form()
		self.ui.setupUi(self)

		self.edit_tab = EditTab(self)
		self.defaults_tab = DefaultsTab(self)
		self.import_tab = ImportTab(self)

		self.last_file_dialog_dir = ProjectContext.project_resource_folder()


TEXTURE_FILE_EXTENSIONS = [
	".png",
	".gif",
	".svg",
	".jpg",
	".jpeg",
	".bmp",
	".tga"
]


class Texture:
	def __init__(self, filepath):
		self.filepath = filepath
		with open(f"{filepath}.oly", 'r') as f:
			self.content = toml.load(f)
		self.slots = self.content['texture']
		self.is_svg = Path(filepath).suffix == ".svg"
		self.is_gif = Path(filepath).suffix == ".gif"
		assert len(self.slots) > 0

	def dump(self):
		with open(f"{self.filepath}.oly", 'w') as f:
			toml.dump(self.content, f)


class EditTab:
	def __init__(self, texture_editor: TextureEditorWidget):
		self.editor = texture_editor
		self.ui = self.editor.ui
		self.texture: Optional[Texture] = None

		self.ui.editTextureBrowse.clicked.connect(self.browse_texture)

		self.ui.editTextureSlotCombo.currentIndexChanged.connect(self.load_texture_slot)
		self.ui.editSpritesheet.checkStateChanged.connect(self.spritesheet_checked_changed)
		self.ui.editTextureNewSlot.clicked.connect(self.add_new_slot)
		self.ui.editTextureDeleteSlot.clicked.connect(self.delete_slot)

		self.ui.applySlotSettings.clicked.connect(self.apply_slot_settings)
		self.ui.cancelSlotSettings.clicked.connect(self.cancel_slot_settings)
		self.ui.resetSlotSettings.clicked.connect(self.reset_slot_settings)

		self.raster_form = SettingsForm([
			SettingsParameter('storage', self.ui.editImageStorage),
			SettingsParameter('min filter', self.ui.editImageMinFilter),
			SettingsParameter('mag filter', self.ui.editImageMagFilter),
			SettingsParameter('generate mipmaps', self.ui.editImageMipmaps),
			SettingsParameter('wrap s', self.ui.editImageWrapS),
			SettingsParameter('wrap t', self.ui.editImageWrapT)
		])

		self.svg_form = SettingsForm([
			SettingsParameter('abstract storage', self.ui.editSVGAbstractStorage),
			SettingsParameter('image storage', self.ui.editSVGImageStorage),
			SettingsParameter('min filter', self.ui.editSVGMinFilter),
			SettingsParameter('mag filter', self.ui.editSVGMagFilter),
			SettingsParameter('generate mipmaps', self.ui.editSVGMipmaps),
			SettingsParameter('wrap s', self.ui.editSVGWrapS),
			SettingsParameter('wrap t', self.ui.editSVGWrapT)
		])

		self.spritesheet_form = SettingsForm([
			SettingsParameter('rows', self.ui.spritesheetRows),
			SettingsParameter('cols', self.ui.spritesheetColumns),
			SettingsParameter('cell width override', self.ui.spritesheetCellWidthOverride),
			SettingsParameter('cell height override', self.ui.spritesheetCellHeightOverride),
			SettingsParameter('delay cs', self.ui.spritesheetDelayCS),
			SettingsParameter('row major', self.ui.spritesheetRowMajor),
			SettingsParameter('row up', self.ui.spritesheetRowUp),
		])

		self.spritesheet_defaults = {
			'rows': 1,
			'cols': 1,
			'cell width override': 0,
			'cell height override': 0,
			'delay cs': 0,
			'row major': True,
			'row up': True
		}

		self.texture_filepath_changed()

	def browse_texture(self):
		filter = FileIO.file_extension_filter("Image files", TEXTURE_FILE_EXTENSIONS)
		filepath, _ = QFileDialog.getOpenFileName(self.editor, "Select File", self.editor.last_file_dialog_dir, filter=filter)
		if filepath:
			self.editor.last_file_dialog_dir = os.path.dirname(filepath)
			self.ui.editTextureFilepath.setText(filepath)
			self.texture_filepath_changed()

	def texture_filepath_changed(self):
		filepath = self.ui.editTextureFilepath.text()
		if len(filepath) > 0:
			if not os.path.exists(f"{filepath}.oly"):
				self.editor.import_tab.create_default_texture_import(filepath)

			try:
				self.texture = Texture(filepath)
			except Exception:
				self.texture = None
				self.ui.paramsLayout.hide()
				self.ui.editTextureDeleteSlot.setDisabled(True)
				QMessageBox.warning(self.editor, "Error", f"Could not load texture {filepath} - the import file is corrupted.")
				return

			self.ui.paramsLayout.show()
			if self.texture.is_svg:
				self.ui.editParams.setCurrentWidget(self.ui.editSVGParams)
			else:
				self.ui.editParams.setCurrentWidget(self.ui.editImageParams)

			self.ui.editTextureSlotCombo.blockSignals(True)
			self.ui.editTextureSlotCombo.clear()
			for i in range(len(self.texture.slots)):
				self.ui.editTextureSlotCombo.addItem(f"{i}")
			self.ui.editTextureSlotCombo.blockSignals(False)

			self.ui.editTextureDeleteSlot.setDisabled(self.ui.editTextureSlotCombo.count() == 1)

			if self.ui.editTextureSlotCombo.currentIndex() == 0:
				self.load_texture_slot()
			else:
				self.ui.editTextureSlotCombo.setCurrentIndex(0)
		else:
			self.ui.paramsLayout.hide()
			self.ui.editTextureDeleteSlot.setDisabled(True)

	def spritesheet_checked_changed(self):
		if self.ui.editSpritesheet.isChecked():
			self.ui.editSpritesheetParams.show()

			slot = self.ui.editTextureSlotCombo.currentIndex()
			tex = self.texture.slots[slot]
			self.spritesheet_form.load_dict(tex, self.spritesheet_defaults)
		else:
			self.ui.editSpritesheetParams.hide()

	def load_texture_slot(self):
		if self.texture is None:
			return

		slot = self.ui.editTextureSlotCombo.currentIndex()
		tex = self.texture.slots[slot]

		# regular settings
		if self.texture.is_svg:
			self.load_svg_dict_into_slot(tex)
		else:
			self.load_raster_dict_into_slot(tex)

		# spritesheet settings
		if self.texture.is_gif:
			self.ui.editSpritesheetWidget.hide()
		else:
			self.ui.editSpritesheetWidget.show()
			if 'anim' in tex and tex['anim']:
				self.ui.editSpritesheetParams.show()
				if self.ui.editSpritesheet.isChecked():
					self.spritesheet_checked_changed()
				else:
					self.ui.editSpritesheet.setChecked(True)
			else:
				self.ui.editSpritesheetParams.hide()
				if self.ui.editSpritesheet.isChecked():
					self.ui.editSpritesheet.setChecked(False)
				else:
					self.spritesheet_checked_changed()

	def load_raster_dict_into_slot(self, tex):
		self.raster_form.load_dict(tex, self.editor.defaults_tab.get_stored_default_raster_dict())

	def load_svg_dict_into_slot(self, tex):
		self.svg_form.load_dict(tex, self.editor.defaults_tab.get_stored_default_svg_dict())

	def add_new_slot(self):
		if self.texture is not None:
			slot = self.ui.editTextureSlotCombo.count()
			self.ui.editTextureSlotCombo.addItem(f"{slot}")
			if self.texture.is_svg:
				self.texture.slots.append(self.editor.defaults_tab.get_stored_default_svg_dict())
			else:
				self.texture.slots.append(self.editor.defaults_tab.get_stored_default_raster_dict())
			self.ui.editTextureSlotCombo.setCurrentIndex(slot)
			self.texture.dump()
			self.ui.editTextureDeleteSlot.setDisabled(False)

	def delete_slot(self):
		if self.texture is not None and len(self.texture.slots) > 1:
			slot = self.ui.editTextureSlotCombo.currentIndex()
			self.texture.slots.pop(slot)
			self.texture.dump()

			for i in range(slot + 1, self.ui.editTextureSlotCombo.count()):
				self.ui.editTextureSlotCombo.setItemText(i, f"{i - 1}")
			self.ui.editTextureSlotCombo.removeItem(slot)
			if self.ui.editTextureSlotCombo.count() == 1:
				self.ui.editTextureDeleteSlot.setDisabled(True)

	def apply_slot_settings(self):
		if self.texture is not None:
			slot = self.ui.editTextureSlotCombo.currentIndex()
			tex = self.texture.slots[slot]

			if self.texture.is_svg:
				tex.update(self.svg_form.get_dict())
			else:
				tex.update(self.raster_form.get_dict())

			if self.ui.editSpritesheet.isChecked():
				assert not self.texture.is_gif
				tex['anim'] = True
				tex.update(self.spritesheet_form.get_dict())
			else:
				tex.pop('anim', None)
				for name in self.spritesheet_form.params:
					tex.pop(name, None)

			self.texture.dump()

	def cancel_slot_settings(self):
		if self.texture is not None:
			self.load_texture_slot()

	def reset_slot_settings(self):
		if self.texture is not None:
			if self.texture.is_svg:
				defaults = self.editor.defaults_tab.get_stored_default_svg_dict()
				self.load_svg_dict_into_slot(defaults)
			else:
				defaults = self.editor.defaults_tab.get_stored_default_raster_dict()
				self.load_raster_dict_into_slot(defaults)


class DefaultsTab:
	def __init__(self, texture_editor: TextureEditorWidget):
		self.editor = texture_editor
		self.ui = self.editor.ui

		self.raster_form = SettingsForm([
			SettingsParameter('storage', self.ui.defaultImageStorage),
			SettingsParameter('min filter', self.ui.defaultImageMinFilter),
			SettingsParameter('mag filter', self.ui.defaultImageMagFilter),
			SettingsParameter('generate mipmaps', self.ui.defaultImageMipmaps),
			SettingsParameter('wrap s', self.ui.defaultImageWrapS),
			SettingsParameter('wrap t', self.ui.defaultImageWrapT)
		])

		self.svg_form = SettingsForm([
			SettingsParameter('abstract storage', self.ui.defaultSVGAbstractStorage),
			SettingsParameter('image storage', self.ui.defaultSVGImageStorage),
			SettingsParameter('min filter', self.ui.defaultSVGMinFilter),
			SettingsParameter('mag filter', self.ui.defaultSVGMagFilter),
			SettingsParameter('generate mipmaps', self.ui.defaultSVGMipmaps),
			SettingsParameter('wrap s', self.ui.defaultSVGWrapS),
			SettingsParameter('wrap t', self.ui.defaultSVGWrapT)
		])

		self.ui.saveDefaultsButton.clicked.connect(self.save_defaults)
		self.ui.cancelDefaultsButton.clicked.connect(self.load_defaults)
		self.validate_project_filepaths()
		self.load_defaults()

	@staticmethod
	def default_raster_texture_filepath():
		project_id = MANIFEST.get_project_id(ProjectContext.PROJECT_FILE)
		return f'projects/{project_id}/asset_defaults/raster_texture.toml'

	@staticmethod
	def default_svg_texture_filepath():
		project_id = MANIFEST.get_project_id(ProjectContext.PROJECT_FILE)
		return f'projects/{project_id}/asset_defaults/svg_texture.toml'

	def validate_project_filepaths(self):
		filepath = self.default_raster_texture_filepath()
		os.makedirs(os.path.dirname(filepath), exist_ok=True)
		if not os.path.exists(filepath):
			self.save_raster_defaults()

		filepath = self.default_svg_texture_filepath()
		os.makedirs(os.path.dirname(filepath), exist_ok=True)
		if not os.path.exists(filepath):
			self.save_svg_defaults()

	def get_stored_default_raster_dict(self):
		with open(self.default_raster_texture_filepath(), 'r') as f:
			return toml.load(f)

	def get_stored_default_svg_dict(self):
		with open(self.default_svg_texture_filepath(), 'r') as f:
			return toml.load(f)

	def load_defaults(self):
		self.raster_form.load_dict(self.get_stored_default_raster_dict())
		self.svg_form.load_dict(self.get_stored_default_svg_dict())

	def save_defaults(self):
		self.save_raster_defaults()
		self.save_svg_defaults()

	def save_raster_defaults(self):
		with open(self.default_raster_texture_filepath(), 'w') as f:
			toml.dump(self.raster_form.get_dict(), f)

	def save_svg_defaults(self):
		with open(self.default_svg_texture_filepath(), 'w') as f:
			toml.dump(self.svg_form.get_dict(), f)


class ImportTab:
	def __init__(self, texture_editor: TextureEditorWidget):
		self.editor = texture_editor
		self.ui = self.editor.ui

		self.ui.importBrowseButton.clicked.connect(self.browse_folder)
		self.ui.executeImportButton.clicked.connect(self.execute)

	def browse_folder(self):
		folder = QFileDialog.getExistingDirectory(self.editor, "Select Folder", self.editor.last_file_dialog_dir)
		if folder:
			self.editor.last_file_dialog_dir = folder
			self.ui.importFolder.setText(folder)

	def execute(self):
		root_folder = self.ui.importFolder.text()
		if len(root_folder) > 0:
			FileIO.execute_standard_import_on_folder(
				root_folder=root_folder,
				recursive=self.ui.importRecursiveSearch.isChecked(),
				clear=self.ui.importClear.isChecked(),
				clean_unused=self.ui.importCleanUnused.isChecked(),
				import_unimported=self.ui.importUnimported.isChecked(),
				extensions=TEXTURE_FILE_EXTENSIONS,
				content_condition=lambda tex: 'texture' in tex and len(tex['texture']) > 0,
				create_default_import=self.create_default_texture_import
			)

	def create_default_texture_import(self, filepath):
		if Path(filepath).suffix == ".svg":
			defaults = self.editor.defaults_tab.get_stored_default_svg_dict()
		else:
			defaults = self.editor.defaults_tab.get_stored_default_raster_dict()
		FileIO.create_default_import_file(filepath, {'texture': [defaults]})
