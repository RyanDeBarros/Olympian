from __future__ import annotations

from pathlib import Path
from typing import TYPE_CHECKING

import send2trash
from PySide6.QtWidgets import QMessageBox

from editor.core import SettingsForm, SettingsParameter
from editor.core.CollapsibleBox import CollapsibleBox

if TYPE_CHECKING:
	from .ProjectSettings import ProjectSettingsTab


class LoggerSettingsManager:
	def __init__(self, tab: ProjectSettingsTab):
		self.tab = tab
		self.ui = self.tab.ui

		self.logger_settings = {}
		self.logger_form = SettingsForm([
			SettingsParameter('logfile', self.ui.logfile),
			SettingsParameter('append', self.ui.appendLogfile),
			SettingsParameter('console', self.ui.logToConsole)
		])

		self.ui.loggerClearLog.clicked.connect(self.clear_logger_log)

		self.ui.loggerStreamsBox = CollapsibleBox.convert_group_box(self.ui.loggerStreamsBox)

		self.logger_enable_settings = {}
		self.logger_enable_form = SettingsForm([
			SettingsParameter('debug', self.ui.logStreamDebugEnable),
			SettingsParameter('info', self.ui.logStreamInfoEnable),
			SettingsParameter('warning', self.ui.logStreamWarningEnable),
			SettingsParameter('error', self.ui.logStreamErrorEnable),
			SettingsParameter('fatal', self.ui.logStreamFatalEnable),
		])

	def connect_modified(self):
		self.logger_form.connect_modified(lambda: self.tab.set_asterisk(True))
		self.logger_enable_form.connect_modified(lambda: self.tab.set_asterisk(True))

	def save(self):
		self.logger_settings.update(self.logger_form.get_dict())
		self.logger_enable_settings.update(self.logger_enable_form.get_dict())

	def load(self):
		if 'logger' not in self.tab.context:
			self.tab.context['logger'] = {}
		self.logger_settings = self.tab.context['logger']
		self.logger_form.load_dict(self.logger_settings)

		if 'enable' not in self.logger_settings:
			self.logger_settings['enable'] = {}
		self.logger_enable_settings = self.logger_settings['enable']
		self.logger_enable_form.load_dict(self.logger_enable_settings)

	def clear_logger_log(self):
		logfile = Path(self.ui.logfile.text())
		if logfile.exists():
			reply = QMessageBox.question(self.tab, "Confirm Action", "Are you sure you want to clear the project log?",
										 defaultButton=QMessageBox.StandardButton.No)
			if reply == QMessageBox.StandardButton.Yes:
				send2trash.send2trash(logfile)
