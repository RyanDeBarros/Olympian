import posixpath
from pathlib import Path


class ProjectContext:
	from editor.core import MainWindow

	def __init__(self, project_file, main_window: MainWindow):
		self.project_file = project_file
		self.project_folder = Path(posixpath.dirname(self.project_file))
		self.res_folder = self.project_folder.joinpath("res")

		self.trash_folder = self.project_folder.joinpath(".trash")
		self.trash_folder.mkdir(exist_ok=True)
		self.settings_folder = self.project_folder.joinpath(".settings")
		self.settings_folder.mkdir(exist_ok=True)
		self.asset_defaults_file = self.settings_folder.joinpath("AssetDefaults.toml")
		self.asset_defaults_file.touch(exist_ok=True)

		self.main_window = main_window
		from editor.core.InputSignalRegistry import InputSignalRegistry
		self.input_signal_registry = InputSignalRegistry(self)

	def refresh(self):
		self.input_signal_registry.load()
