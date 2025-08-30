from dataclasses import dataclass
from pathlib import Path


@dataclass
class AssetDefaultsDirectory:
	folder_path: Path
	texture_file: Path
	font_file: Path

	def touch(self):
		self.texture_file.touch(exist_ok=True)
		self.font_file.touch(exist_ok=True)


class ProjectContext:
	from editor.core import MainWindow

	def __init__(self, project_file, main_window: MainWindow):
		self.project_file = Path(project_file).resolve()
		self.project_folder = self.project_file.parent
		self.res_folder = self.project_folder.joinpath("res")
		self.res_friendly_prefix = "RES://"

		self.trash_folder = self.project_folder.joinpath(".trash")
		self.trash_folder.mkdir(exist_ok=True)
		self.settings_folder = self.project_folder.joinpath(".settings")
		self.settings_folder.mkdir(exist_ok=True)

		self.asset_defaults_directory = AssetDefaultsDirectory(
			folder_path=self.settings_folder,
			texture_file=self.settings_folder.joinpath("TextureDefaults.toml"),
			font_file=self.settings_folder.joinpath("FontDefaults.toml")
		)
		self.asset_defaults_directory.touch()

		self.favorites_file = self.settings_folder.joinpath("ResFavorites.toml")
		self.favorites_file.touch()

		self.main_window = main_window
		from editor.core.InputSignalRegistry import InputSignalRegistry
		self.input_signal_registry = InputSignalRegistry(self)

	def refresh(self):
		self.input_signal_registry.load()

	def to_friendly_resource_path(self, resource: Path) -> str:
		relative = resource.relative_to(self.res_folder)
		return self.res_friendly_prefix + (relative.as_posix() if str(relative) != "." else "")

	def from_friendly_resource_path(self, resource: str) -> Path:
		relative = resource[len(self.res_friendly_prefix):]
		if relative == "":
			relative = "."
		return self.res_folder.joinpath(Path(relative))
