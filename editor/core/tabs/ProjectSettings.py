from typing import override

import toml
from PySide6.QtCore import QSize
from PySide6.QtGui import QIcon, QPixmap

from editor import ui
from editor.core import MainWindow, AbstractPathItem
from editor.core.MainTabHolder import EditorTab
from editor.core.common import SettingsForm, SettingsParameter


class ProjectSettingsTab(EditorTab):
	class UID:
		def __eq__(self, other):
			return isinstance(other, ProjectSettingsTab.UID)

	def __init__(self, win: MainWindow):
		super().__init__(win)

		self.settings = {}
		self.context = {}

		self.ui = ui.ProjectSettings.Ui_ProjectSettings()
		self.ui.setupUi(self)

		self.window_settings = {}
		self.window_form = SettingsForm([
			SettingsParameter('width', self.ui.windowWidth),
			SettingsParameter('height', self.ui.windowHeight),
			SettingsParameter('title', self.ui.windowTitle)
		])

		self.logger_settings = {}
		self.logger_form = SettingsForm([
			SettingsParameter('logfile', self.ui.logfile),
			SettingsParameter('append', self.ui.appendLogfile),
			SettingsParameter('console', self.ui.logToConsole)
		])

		self.revert_changes_impl()
		self.window_form.connect_modified(lambda: self.set_asterisk(True))
		self.logger_form.connect_modified(lambda: self.set_asterisk(True))

	@override
	def uid(self):
		return ProjectSettingsTab.UID()

	@override
	def icon(self, size: QSize):
		return QIcon(QPixmap("res/images/Gear.png").scaled(size))

	@override
	def name(self):
		return "Project Settings"

	@override
	def save_changes_impl(self):
		self.window_settings.update(self.window_form.get_dict())
		self.logger_settings.update(self.logger_form.get_dict())

		with open(self.win.project_context.project_file, 'w') as f:
			toml.dump(self.settings, f)

	@override
	def revert_changes_impl(self):
		with open(self.win.project_context.project_file, 'r') as f:
			self.settings = toml.load(f)

		assert 'context' in self.settings
		self.context = self.settings['context']
		assert 'window' in self.context
		self.window_settings = self.context['window']
		self.window_form.load_dict(self.window_settings)
		assert 'logger' in self.context
		self.logger_settings = self.context['logger']
		self.logger_form.load_dict(self.logger_settings)

	@override
	def rename_impl(self, item: AbstractPathItem):
		raise RuntimeError("Project settings tab cannot be renamed")
