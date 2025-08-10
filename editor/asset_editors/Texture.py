import os
import posixpath
from pathlib import Path
from typing import List, Optional

import toml
from PySide6.QtWidgets import QWidget, QFileDialog, QMessageBox

from editor import ui, MANIFEST, ProjectContext
from editor import FileIO
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
			self.slots: List[dict] = toml.load(f)['texture']
		self.is_svg = Path(filepath).suffix == ".svg"
		self.is_gif = Path(filepath).suffix == ".gif"

	def dump(self):
		with open(f"{self.filepath}.oly", 'w') as f:
			toml.dump({'texture': self.slots}, f)


class EditTab:
	def __init__(self, texture_editor: TextureEditorWidget):
		self.editor = texture_editor
		self.ui = self.editor.ui
		self.texture: Optional[Texture] = None

		self.ui.editTextureBrowse.clicked.connect(self.browse_texture)
		self.ui.editTextureFilepath.textChanged.connect(self.texture_filepath_changed)

		self.ui.editTextureSlotCombo.currentIndexChanged.connect(self.load_texture_slot)
		self.ui.editSpritesheet.checkStateChanged.connect(self.spritesheet_checked_changed)
		self.ui.editTextureNewSlot.clicked.connect(self.add_new_slot)
		self.ui.editTextureDeleteSlot.clicked.connect(self.delete_slot)

		self.ui.applySlotSettings.clicked.connect(self.apply_slot_settings)
		self.ui.cancelSlotSettings.clicked.connect(self.cancel_slot_settings)
		self.ui.resetSlotSettings.clicked.connect(self.reset_slot_settings)

		self.texture_filepath_changed()

	def browse_texture(self):
		filter = f"Image files ({" ".join([f"*{ext}" for ext in TEXTURE_FILE_EXTENSIONS])})"
		filepath, _ = QFileDialog.getOpenFileName(self.editor, "Select File", self.editor.last_file_dialog_dir, filter=filter)
		if filepath:
			self.editor.last_file_dialog_dir = os.path.dirname(filepath)
			if self.ui.editTextureFilepath.text() == filepath:
				self.texture_filepath_changed()
			else:
				self.ui.editTextureFilepath.setText(filepath)

	def texture_filepath_changed(self):
		filepath = self.ui.editTextureFilepath.text()
		if len(filepath) > 0:
			if not os.path.exists(f"{filepath}.oly"):
				self.editor.import_tab.create_default_texture_import(filepath)

			try:
				self.texture = Texture(filepath)
				assert len(self.texture.slots) >= 1
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

			self.ui.spritesheetRows.setValue(tex.get('rows', 1))
			self.ui.spritesheetColumns.setValue(tex.get('cols', 1))
			self.ui.spritesheetCellWidthOverride.setValue(tex.get('cell width override', 0))
			self.ui.spritesheetCellHeightOverride.setValue(tex.get('cell height override', 0))
			self.ui.spritesheetDelayCS.setValue(tex.get('delay cs', 0))
			self.ui.spritesheetRowMajor.setChecked(tex.get('row major', True))
			self.ui.spritesheetRowUp.setChecked(tex.get('row up', True))
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
		defaults = self.editor.defaults_tab.get_stored_default_raster_dict()
		self.ui.editImageStorage.setCurrentText(PARAM_LIST.get_name(tex.get('storage', defaults['storage'])))
		self.ui.editImageMinFilter.setCurrentText(PARAM_LIST.get_name(tex.get('min filter', defaults['min filter'])))
		self.ui.editImageMagFilter.setCurrentText(PARAM_LIST.get_name(tex.get('mag filter', defaults['mag filter'])))
		self.ui.editImageMipmaps.setChecked(tex.get('generate mipmaps', defaults['generate mipmaps']))
		self.ui.editImageWrapS.setCurrentText(PARAM_LIST.get_name(tex.get('wrap s', defaults['wrap s'])))
		self.ui.editImageWrapT.setCurrentText(PARAM_LIST.get_name(tex.get('wrap t', defaults['wrap t'])))

	def load_svg_dict_into_slot(self, tex):
		defaults = self.editor.defaults_tab.get_stored_default_svg_dict()
		self.ui.editSVGAbstractStorage.setCurrentText(PARAM_LIST.get_name(tex.get('abstract storage', defaults['abstract storage'])))
		self.ui.editSVGImageStorage.setCurrentText(PARAM_LIST.get_name(tex.get('image storage', defaults['image storage'])))
		self.ui.editSVGMinFilter.setCurrentText(PARAM_LIST.get_name(tex.get('min filter', defaults['min filter'])))
		self.ui.editSVGMagFilter.setCurrentText(PARAM_LIST.get_name(tex.get('mag filter', defaults['mag filter'])))
		self.ui.editSVGMipmaps.setCurrentText(PARAM_LIST.get_name(tex.get('generate mipmaps', defaults['generate mipmaps'])))
		self.ui.editSVGWrapS.setCurrentText(PARAM_LIST.get_name(tex.get('wrap s', defaults['wrap s'])))
		self.ui.editSVGWrapT.setCurrentText(PARAM_LIST.get_name(tex.get('wrap t', defaults['wrap t'])))

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
				tex['abstract storage'] = PARAM_LIST.get_value(self.ui.editSVGAbstractStorage.currentText())
				tex['image storage'] = PARAM_LIST.get_value(self.ui.editSVGImageStorage.currentText())
				tex['min filter'] = PARAM_LIST.get_value(self.ui.defaultSVGMinFilter.currentText())
				tex['mag filter'] = PARAM_LIST.get_value(self.ui.defaultSVGMagFilter.currentText())
				tex['generate mipmaps'] = PARAM_LIST.get_value(self.ui.editSVGMipmaps.currentText())
				tex['wrap s'] = PARAM_LIST.get_value(self.ui.editSVGWrapS.currentText())
				tex['wrap t'] = PARAM_LIST.get_value(self.ui.editSVGWrapT.currentText())
			else:
				tex['storage'] = PARAM_LIST.get_value(self.ui.editImageStorage.currentText())
				tex['min filter'] = PARAM_LIST.get_value(self.ui.editImageMinFilter.currentText())
				tex['mag filter'] = PARAM_LIST.get_value(self.ui.editImageMagFilter.currentText())
				tex['generate mipmaps'] = self.ui.editImageMipmaps.isChecked()
				tex['wrap s'] = PARAM_LIST.get_value(self.ui.editImageWrapS.currentText())
				tex['wrap t'] = PARAM_LIST.get_value(self.ui.editImageWrapT.currentText())

			if self.ui.editSpritesheet.isChecked():
				assert not self.texture.is_gif
				tex['anim'] = True
				tex['rows'] = self.ui.spritesheetRows.value()
				tex['cols'] = self.ui.spritesheetColumns.value()
				tex['cell width override'] = self.ui.spritesheetCellWidthOverride.value()
				tex['cell height override'] = self.ui.spritesheetCellHeightOverride.value()
				tex['delay cs'] = self.ui.spritesheetDelayCS.value()
				tex['row major'] = self.ui.spritesheetRowMajor.isChecked()
				tex['row up'] = self.ui.spritesheetRowUp.isChecked()
			else:
				tex.pop('anim', None)
				tex.pop('rows', None)
				tex.pop('cols', None)
				tex.pop('cell width override', None)
				tex.pop('cell height override', None)
				tex.pop('delay cs', None)
				tex.pop('row major', None)
				tex.pop('row up', None)

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


# TODO v3 allow selection of specific (even multiple) files, rather than folder
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

	def get_texture_files(self, folder):
		files = set()
		for entry in os.listdir(folder):
			path = os.path.join(folder, entry)
			if os.path.isfile(path):
				_, ext = os.path.splitext(path)
				if ext in TEXTURE_FILE_EXTENSIONS:
					files.add(path)
		return files

	def get_texture_imports(self, folder):
		imports = set()
		for entry in os.listdir(folder):
			path = os.path.join(folder, entry)
			if os.path.isfile(path) and path.endswith(".oly"):
				_, ext = os.path.splitext(path[:-len(".oly")])
				if ext in TEXTURE_FILE_EXTENSIONS:
					try:
						with open(path, 'r') as f:
							tex = toml.load(f)
						if 'texture' in tex and len(tex['texture']) > 0:
							imports.add(path)
					except toml.TomlDecodeError:
						pass
		return imports

	def execute(self):
		if len(self.ui.importFolder.text()) == 0:
			return

		root_folder = self.ui.importFolder.text()
		recursive = self.ui.importRecursiveSearch.isChecked()
		clear = self.ui.importClear.isChecked()
		clean_unused = self.ui.importCleanUnused.isChecked()
		import_unimported = self.ui.importUnimported.isChecked()

		def execute_on_folder(folder):
			if clear:
				texture_imports = self.get_texture_imports(folder)
				for texture_import in texture_imports:
					FileIO.move_to_trash(texture_import)
			elif clean_unused:
				texture_imports = self.get_texture_imports(folder)
				for texture_import in texture_imports:
					if not os.path.exists(texture_import[:-len(".oly")]):
						FileIO.move_to_trash(texture_import)
			if import_unimported:
				texture_files = self.get_texture_files(folder)
				for texture_file in texture_files:
					if not self.has_texture_import(texture_file):
						self.create_default_texture_import(texture_file)

		if recursive:
			for root, dirs, files in os.walk(root_folder):
				execute_on_folder(root)
		else:
			execute_on_folder(root_folder)

	def has_texture_import(self, filepath):
		if not os.path.exists(f"{filepath}.oly"):
			return False
		try:
			with open(f"{filepath}.oly", 'r') as f:
				tex = toml.load(f)
				return 'texture' in tex and len(tex['texture']) > 0
		except toml.TomlDecodeError:
			return False

	def create_default_texture_import(self, filepath):
		if Path(filepath).suffix == ".svg":
			defaults = self.editor.defaults_tab.get_stored_default_svg_dict()
		else:
			defaults = self.editor.defaults_tab.get_stored_default_raster_dict()

		with open(f"{filepath}.oly", 'w') as f:
			toml.dump({'texture': [defaults]}, f)
