from typing import override

import toml
from PySide6.QtCore import QSize
from PySide6.QtGui import QPixmap, QIcon

from editor import ui
from editor.core import PREFERENCES, MainWindow, AbstractPathItem, SettingsParameter, SettingsForm
from editor.core.MainTabHolder import EditorTab


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
		return QIcon(QPixmap("res/images/File.png").scaled(size))  # TODO v3 editor preferences icon

	@override
	def name(self):
		return "Editor Preferences"

	@override
	def save_changes_impl(self):
		self.nav_settings.update(self.nav_form.get_dict())
		PREFERENCES.prompt_user_when_deleting_paths = self.nav_settings['prompt on delete']

		with open('data/PREFERENCES.toml', 'w') as f:
			toml.dump(self.preferences, f)

	@override
	def revert_changes_impl(self):
		with open('data/PREFERENCES.toml', 'r') as f:
			self.preferences = toml.load(f)

		if 'navigation' not in self.preferences:
			self.preferences['navigation'] = {}
		self.nav_settings = self.preferences['navigation']
		self.nav_form.load_dict(self.nav_settings, {
			'prompt on delete': True
		})

	@override
	def rename_impl(self, item: AbstractPathItem):
		raise RuntimeError("Editor preferences tab cannot be renamed")
