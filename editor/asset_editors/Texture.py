import toml
from PySide6.QtWidgets import QWidget

from editor import ui


class TextureEditorWidget(QWidget):
	def __init__(self, win):
		super().__init__()
		self.win = win

		self.ui = ui.asset_editors.Texture.Ui_Form()
		self.ui.setupUi(self)

		self.edit_tab = EditTab(self)
		self.defaults_tab = DefaultsTab(self)
		self.import_tab = ImportTab(self)


class EditTab:
	def __init__(self, texture_editor: TextureEditorWidget):
		self.editor = texture_editor
		self.texture = {}


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
			'min filter': self.ui.defaultImageMinFilter.currentText().lower(),
			'mag filter': self.ui.defaultImageMagFilter.currentText().lower(),
			'generate mipmaps': self.ui.defaultImageMipmaps.isChecked(),
			'wrap s': self.ui.defaultImageWrapS.currentText().lower(),
			'wrap t': self.ui.defaultImageWrapT.currentText().lower()
		}

	def get_stored_default_svg_dict(self):
		with open('data/asset_defaults/svg_texture.toml', 'r') as f:
			return toml.load(f)

	def _get_default_svg_dict(self):
		return {
			'abstract storage': self.ui.defaultSVGAbstractStorage.currentText().lower(),
			'image storage': self.ui.defaultSVGImageStorage.currentText().lower(),
			'min filter': self.ui.defaultSVGMinFilter.currentText().lower(),
			'mag filter': self.ui.defaultSVGMagFilter.currentText().lower(),
			'generate mipmaps': self.ui.defaultSVGMipmaps.currentText().lower(),
			'wrap s': self.ui.defaultSVGWrapS.currentText().lower(),
			'wrap t': self.ui.defaultSVGWrapT.currentText().lower()
		}

	def load_defaults(self):
		defaults = self.get_stored_default_raster_dict()
		self.ui.defaultImageStorage.setCurrentText(defaults['storage'].title())
		self.ui.defaultImageMinFilter.setCurrentText(defaults['min filter'].title())
		self.ui.defaultImageMagFilter.setCurrentText(defaults['mag filter'].title())
		self.ui.defaultImageMipmaps.setChecked(defaults['generate mipmaps'])
		self.ui.defaultImageWrapS.setCurrentText(defaults['wrap s'].title())
		self.ui.defaultImageWrapT.setCurrentText(defaults['wrap t'].title())

		defaults = self.get_stored_default_svg_dict()
		self.ui.defaultSVGAbstractStorage.setCurrentText(defaults['abstract storage'].title())
		self.ui.defaultSVGImageStorage.setCurrentText(defaults['image storage'].title())
		self.ui.defaultSVGMinFilter.setCurrentText(defaults['min filter'].title())
		self.ui.defaultSVGMagFilter.setCurrentText(defaults['mag filter'].title())
		self.ui.defaultSVGMipmaps.setCurrentText(defaults['generate mipmaps'].title())
		self.ui.defaultSVGWrapS.setCurrentText(defaults['wrap s'].title())
		self.ui.defaultSVGWrapT.setCurrentText(defaults['wrap t'].title())

	def save_defaults(self):
		with open('data/asset_defaults/raster_texture.toml', 'w') as f:
			toml.dump(self._get_default_raster_dict(), f)

		with open('data/asset_defaults/svg_texture.toml', 'w') as f:
			toml.dump(self._get_default_svg_dict(), f)


class ImportTab:
	def __init__(self, texture_editor: TextureEditorWidget):
		self.editor = texture_editor
