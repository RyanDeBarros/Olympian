import os
import re
from pathlib import Path

import send2trash

from editor.core import FileDialog
from editor.tools import TOMLAdapter


class EditorManifest:
	def __init__(self):
		self.manifest_filepath = Path('projects/manifest.toml').resolve()
		self.toml = TOMLAdapter.load(self.manifest_filepath)

		self.project_context_h = Path('data/PROJECT_CONTEXT_H').read_text()
		self.project_context_cpp = Path('data/PROJECT_CONTEXT_CPP').read_text()

		self.browse_file_dialog = FileDialog(Path(self.toml.get('last_file_dialog_dir', os.getcwd())))

	def dump(self):
		self.toml['last_file_dialog_dir'] = self.browse_file_dialog.last_dir.as_posix()
		TOMLAdapter.dump(self.manifest_filepath, self.toml)

	@staticmethod
	def get_default_project_context(project_folder: Path, project_name):
		logfile = project_folder / f"{project_name}.log"
		content = Path('../data/DEFAULT_PROJECT_CONTEXT.toml').read_text()
		return re.sub(r'\{\{LOGFILE}}', logfile.as_posix(), content)

	def project_dict(self) -> dict:
		if 'projects' not in self.toml:
			self.toml['projects'] = []
		return self.toml['projects']

	def project_id_stack(self) -> list:
		if 'project_id_stack' not in self.toml:
			self.toml['project_id_stack'] = []
		return self.toml['project_id_stack']

	def get_project_id(self, project_file: Path) -> int:
		return self.project_dict()[project_file.as_posix()]

	def recent_list(self) -> list:
		if 'recent' not in self.toml:
			self.toml['recent'] = []
		return self.toml['recent']

	def is_filepath_relative_to_existing_project(self, filepath: Path) -> bool:
		folder = filepath.parent
		for project_filepath in self.project_dict():
			project_folder = Path(project_filepath).parent
			if folder.is_relative_to(project_folder) or Path(project_folder).is_relative_to(folder):
				return True
		return False

	def push_to_top_of_recent(self, project_filepath: Path):
		if project_filepath.as_posix() in self.recent_list():
			self.recent_list().remove(project_filepath.as_posix())
		self.recent_list().insert(0, project_filepath.as_posix())
		self.dump()

	def is_valid_project_file(self, project_filepath: Path) -> bool:
		return project_filepath.as_posix() in self.project_dict()

	def get_recent_project_filepaths(self) -> list[str]:
		return self.recent_list()

	@staticmethod
	def get_project_file(project_folder: Path, project_name) -> Path:
		return project_folder / f"{project_name}.oly"

	@staticmethod
	def get_src_folder(project_folder: Path) -> Path:
		return project_folder / "src"

	@staticmethod
	def get_res_folder(project_folder: Path) -> Path:
		return project_folder / "res"

	@staticmethod
	def get_gen_folder(project_folder: Path) -> Path:
		return project_folder / ".gen"

	def create_project(self, project_filepath: Path):
		project_folder = project_filepath.parent
		project_name = project_filepath.stem
		project_folder.mkdir(parents=True, exist_ok=True)
		src_folder = self.get_src_folder(project_folder)
		src_folder.mkdir(parents=True, exist_ok=True)
		self.get_res_folder(project_folder).mkdir(parents=True, exist_ok=True)
		self.get_gen_folder(project_folder).mkdir(parents=True, exist_ok=True)

		project_filepath.write_text(self.get_default_project_context(project_folder, project_name))
		(src_folder / "ProjectContext.h").write_text(self.project_context_h)
		(src_folder / "ProjectContext.cpp").write_text(self.project_context_cpp)

		# TODO v5 create project CMakeLists.txt, LICENSE

		if len(self.project_id_stack()) == 0:
			if 'project next id' not in self.toml:
				self.toml['project next id'] = 1
			project_id = self.toml['project next id']
			self.toml['project next id'] = project_id + 1
		else:
			project_id = self.project_id_stack().pop()

		self.project_dict()[project_filepath.as_posix()] = project_id
		self.dump()

	def remove_project_id(self, project_id):
		self.project_id_stack().append(project_id)
		send2trash.send2trash(f"projects/{project_id}")

	def delete_project(self, project_filepath: Path):
		project_posix = project_filepath.as_posix()
		if project_posix in self.project_dict():
			project_id = self.project_dict()[project_posix]
			self.remove_project_id(project_id)
			del self.project_dict()[project_posix]
			self.recent_list().remove(project_posix)
			self.dump()

	def remove_nonexistent_projects(self):
		keep = []
		removed = []
		for project_filepath, project_id in self.project_dict().items():
			project_filepath = Path(project_filepath).resolve()
			if project_filepath.exists():
				keep.append(project_filepath.as_posix())
			else:
				self.remove_project_id(project_id)
				removed.append(project_filepath.as_posix())
				self.recent_list().remove(project_filepath.as_posix())
		self.toml['projects'] = keep
		self.dump()
		return removed
