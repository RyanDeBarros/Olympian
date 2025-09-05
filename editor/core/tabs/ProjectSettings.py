from pathlib import Path
from typing import override

from PySide6.QtCore import QSize
import send2trash
from PySide6.QtWidgets import QMessageBox

from editor import ui, TOMLAdapter
from editor.core import MainWindow, AbstractPathItem, nice_icon
from editor.core.common import SettingsForm, SettingsParameter
from .EditorTab import EditorTab


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
		# TODO v4 window hints. Eventually use collapsible sections.

		self.viewport_settings = {}
		self.viewport_form = SettingsForm([
			SettingsParameter('boxed', self.ui.viewportBoxed),
			SettingsParameter('stretch', self.ui.viewportStretch)
		])

		self.logger_settings = {}
		self.logger_form = SettingsForm([
			SettingsParameter('logfile', self.ui.logfile),
			SettingsParameter('append', self.ui.appendLogfile),
			SettingsParameter('console', self.ui.logToConsole)
		])

		self.ui.loggerClearLog.clicked.connect(self.clear_logger_log)

		# TODO v4 nested SettingsForms
		self.logger_enable_settings = {}
		self.logger_enable_form = SettingsForm([
			SettingsParameter('debug', self.ui.logStreamDebugEnable),
			SettingsParameter('info', self.ui.logStreamInfoEnable),
			SettingsParameter('warning', self.ui.logStreamWarningEnable),
			SettingsParameter('error', self.ui.logStreamErrorEnable),
			SettingsParameter('fatal', self.ui.logStreamFatalEnable),
		])

		self.revert_changes_impl()
		self.window_form.connect_modified(lambda: self.set_asterisk(True))
		self.viewport_form.connect_modified(lambda: self.set_asterisk(True))
		self.logger_form.connect_modified(lambda: self.set_asterisk(True))
		self.logger_enable_form.connect_modified(lambda: self.set_asterisk(True))  # TODO v4 with nested SettingsForm, logger_enable_form should already be connected due to logger_form being connected.

	@override
	def uid(self):
		return ProjectSettingsTab.UID()

	@override
	def icon(self, size: QSize):
		return nice_icon("res/images/Gear.png", size)

	@override
	def name(self):
		return "Project Settings"

	@override
	def save_changes_impl(self):
		self.window_settings.update(self.window_form.get_dict())
		self.viewport_settings.update(self.viewport_form.get_dict())
		self.logger_settings.update(self.logger_form.get_dict())
		self.logger_enable_settings.update(self.logger_enable_form.get_dict())
		TOMLAdapter.dump(self.win.project_context.project_file, self.settings)

	@override
	def revert_changes_impl(self):
		self.settings = TOMLAdapter.load(self.win.project_context.project_file)

		if 'context' not in self.settings:
			self.settings['context'] = {}
		self.context = self.settings['context']

		if 'window' not in self.context:
			self.context['window'] = {}
		self.window_settings = self.context['window']
		self.window_form.load_dict(self.window_settings)

		if 'viewport' not in self.window_settings:
			self.window_settings['viewport'] = {}
		self.viewport_settings = self.window_settings['viewport']
		self.viewport_form.load_dict(self.viewport_settings)

		if 'logger' not in self.context:
			self.context['logger'] = {}
		self.logger_settings = self.context['logger']
		self.logger_form.load_dict(self.logger_settings)

		if 'enable' not in self.logger_settings:
			self.logger_settings['enable'] = {}
		self.logger_enable_settings = self.logger_settings['enable']
		self.logger_enable_form.load_dict(self.logger_enable_settings)

	@override
	def rename_impl(self, item: AbstractPathItem):
		raise RuntimeError("Project settings tab cannot be renamed")

	@override
	def refresh_impl(self):
		pass

	def clear_logger_log(self):
		logfile = Path(self.ui.logfile.text())
		if logfile.exists():
			reply = QMessageBox.question(self, "Confirm Action", "Are you sure you want to clear the project log?",
										 defaultButton=QMessageBox.StandardButton.No)
			if reply == QMessageBox.StandardButton.Yes:
				send2trash.send2trash(logfile)
