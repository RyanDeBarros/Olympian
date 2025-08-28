from typing import override

from PySide6.QtCore import QSize

from editor import ui, TOMLAdapter
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
			SettingsParameter('prompt on delete', self.ui.navPromptOnDelete)
		])

		self.revert_changes_impl()
		self.nav_form.connect_modified(lambda: self.set_asterisk(True))

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
		PREFERENCES.prompt_user_when_deleting_paths = self.nav_settings['prompt on delete']
		TOMLAdapter.dump('data/PREFERENCES.toml', self.preferences)

	@override
	def revert_changes_impl(self):
		self.preferences = TOMLAdapter.load('data/PREFERENCES.toml')

		if 'navigation' not in self.preferences:
			self.preferences['navigation'] = {}
		self.nav_settings = self.preferences['navigation']
		self.nav_form.load_dict(self.nav_settings, {
			'prompt on delete': True
		})

	@override
	def rename_impl(self, item: AbstractPathItem):
		raise RuntimeError("Editor preferences tab cannot be renamed")

	@override
	def refresh_impl(self):
		pass
