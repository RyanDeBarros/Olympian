from typing import override

from PySide6.QtCore import QSize

from editor import ui
from editor.tools import TOMLAdapter
from editor.core import MainWindow, AbstractPathItem, nice_icon
from .LoggerSettings import LoggerSettingsManager
from .PhysicsSettings import PhysicsSettingsManager
from .SimulationSettings import SimulationSettingsManager
from .WindowSettings import WindowSettingsManager
from ..EditorTab import EditorTab


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
		self.window_settings_manager = WindowSettingsManager(self)
		self.logger_settings_manager = LoggerSettingsManager(self)
		self.simulation_settings_manager = SimulationSettingsManager(self)
		self.physics_settings_manager = PhysicsSettingsManager(self)

		self.revert_changes_impl()
		self.window_settings_manager.connect_modified()
		self.logger_settings_manager.connect_modified()
		self.simulation_settings_manager.connect_modified()
		self.physics_settings_manager.connect_modified()

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
		self.window_settings_manager.save()
		self.logger_settings_manager.save()
		self.simulation_settings_manager.save()
		self.physics_settings_manager.save()

		TOMLAdapter.dump(self.win.project_context.project_file, self.settings)

	@override
	def revert_changes_impl(self):
		self.settings = TOMLAdapter.load(self.win.project_context.project_file)
		if 'context' not in self.settings:
			self.settings['context'] = {}
		self.context = self.settings['context']

		self.window_settings_manager.load()
		self.logger_settings_manager.load()
		self.simulation_settings_manager.load()
		self.physics_settings_manager.load()

	@override
	def rename_impl(self, item: AbstractPathItem):
		raise RuntimeError("Project settings tab cannot be renamed")

	@override
	def refresh_impl(self):
		pass
