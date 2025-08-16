import toml
from PySide6.QtWidgets import QWidget

from editor import ui
from editor.util import ProjectContext
from .Common import SettingsForm, SettingsParameter


# TODO v3 indicate unsaved changes - prompt modal if user tries to change assets or close editor with unsaved changes
# TODO v3 key shortcuts like CTRL+S to save
class ProjectSettingsWidget(QWidget):
	def __init__(self, win):
		super().__init__()
		self.win = win

		self.ui = ui.ProjectSettings.Ui_Form()
		self.ui.setupUi(self)

		self.window_form = SettingsForm([
			SettingsParameter('width', self.ui.windowWidth),
			SettingsParameter('height', self.ui.windowHeight),
			SettingsParameter('title', self.ui.windowTitle)
		])

		self.logger_form = SettingsForm([
			SettingsParameter('logfile', self.ui.logfile),
			SettingsParameter('append', self.ui.appendLogfile),
			SettingsParameter('console', self.ui.logToConsole)
		])

		self.ui.applyChanges.clicked.connect(self.apply_settings)
		self.ui.cancelChanges.clicked.connect(self.cancel_settings)

		with open(ProjectContext.PROJECT_FILE, 'r') as f:
			self.settings = toml.load(f)
		assert 'context' in self.settings
		self.context = self.settings['context']
		assert 'window' in self.context
		self.window_settings = self.context['window']
		assert 'logger' in self.context
		self.logger_settings = self.context['logger']

		self.last_file_dialog_dir = ProjectContext.project_resource_folder()

		self.cancel_settings()

	def cancel_settings(self):
		self.window_form.load_dict(self.window_settings)
		self.logger_form.load_dict(self.logger_settings)

	def apply_settings(self):
		self.window_settings = self.window_form.get_dict()
		self.logger_settings = self.logger_form.get_dict()
		with open(ProjectContext.PROJECT_FILE, 'w') as f:
			toml.dump(self.settings, f)
