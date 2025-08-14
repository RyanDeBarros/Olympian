import toml
from PySide6.QtWidgets import QWidget

from editor import ui
from editor.asset_editors.Common import SettingsForm, SettingsParameter
from editor.util import ProjectContext


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

		self.last_file_dialog_dir = ProjectContext.project_resource_folder()

		self.cancel_settings()

	def cancel_settings(self):
		self.window_form.load_dict(self.settings['context']['window'])
		self.logger_form.load_dict(self.settings['context']['logger'])

	def apply_settings(self):
		self.settings['context']['window'] = self.window_form.get_dict()
		self.settings['context']['logger'] = self.logger_form.get_dict()
		with open(ProjectContext.PROJECT_FILE, 'w') as f:
			toml.dump(self.settings, f)
