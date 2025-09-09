from typing import override

from PySide6.QtCore import QSize

from editor import ui
from ...tools import TOMLAdapter
from editor.core import MainWindow, AbstractPathItem, SettingsParameter, SettingsForm, nice_icon
from editor.core.EditorPreferences import PREFERENCES
from .EditorTab import EditorTab


class EditorPreferencesTab(EditorTab):
	class UID:
		def __eq__(self, other):
			return isinstance(other, EditorPreferencesTab.UID)

	def __init__(self, win: MainWindow):
		super().__init__(win)

		self.preferences = {}

		self.ui = ui.EditorPreferences.Ui_EditorPreferences()
		self.ui.setupUi(self)

		self.nav_settings = {}
		self.nav_form = SettingsForm([
			SettingsParameter('prompt_on_delete', self.ui.navPromptOnDelete)
		])

		self.import_settings = {}
		self.import_form = SettingsForm([
			SettingsParameter('texture_file_extensions', self.ui.textureFileExtensions),
			SettingsParameter('font_file_extensions', self.ui.fontFileExtensions)
		])

		self.revert_changes_impl()
		self.nav_form.connect_modified(lambda: self.set_asterisk(True))
		self.import_form.connect_modified(lambda: self.set_asterisk(True))

	@override
	def uid(self):
		return EditorPreferencesTab.UID()

	@override
	def icon(self, size: QSize):
		return nice_icon("res/images/Gear.png", size)

	@override
	def name(self):
		return "Editor Preferences"

	@override
	def save_changes_impl(self):
		self.nav_settings.update(self.nav_form.get_dict())
		PREFERENCES.prompt_user_when_deleting_paths = self.nav_settings['prompt_on_delete']
		self.import_settings.update(self.import_form.get_dict())
		PREFERENCES.texture_file_extensions = self.import_settings['texture_file_extensions'].split(';')
		PREFERENCES.font_file_extensions = self.import_settings['font_file_extensions'].split(';')
		TOMLAdapter.dump('data/PREFERENCES.toml', self.preferences)

	@override
	def revert_changes_impl(self):
		self.preferences = TOMLAdapter.load('data/PREFERENCES.toml')

		if 'navigation' not in self.preferences:
			self.preferences['navigation'] = {}
		self.nav_settings = self.preferences['navigation']
		self.nav_form.load_dict(self.nav_settings, {
			'prompt_on_delete': True
		})

		if 'file_imports' not in self.preferences:
			self.preferences['file_imports'] = {}
		self.import_settings = self.preferences['file_imports']
		self.import_form.load_dict(self.import_settings, {
			'texture_file_extensions': '.png;.jpg;.jpeg;.bmp;.gif;.svg',
			'font_file_extensions': '.ttf;.otf'
		})

	@override
	def rename_impl(self, item: AbstractPathItem):
		raise RuntimeError("Editor preferences tab cannot be renamed")

	@override
	def refresh_impl(self):
		pass
