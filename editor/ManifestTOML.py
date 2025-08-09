import os
import re
from pathlib import Path
import posixpath

import toml
import send2trash


class ManifestTOML:
	def __init__(self):
		self.toml_filepath = 'projects/manifest.toml'
		with open(self.toml_filepath, 'r') as f:
			self.toml = toml.load(f)

		with open('data/PROJECT_CONTEXT_H', 'r') as f:
			self.project_context_h = f.read()

		with open('data/PROJECT_CONTEXT_CPP', 'r') as f:
			self.project_context_cpp = f.read()

	def dump(self):
		with open(self.toml_filepath, 'w') as f:
			toml.dump(self.toml, f)

	def get_default_project_context(self, project_folder, project_name):
		logfile = posixpath.join(project_folder, f"{project_name}.log")
		with open('data/DEFAULT_PROJECT_CONTEXT.toml', 'r') as f:
			content = f.read()
		return re.sub(r'\{\{LOGFILE}}', logfile, content)

	def get_last_file_dialog_dir(self):
		folder = self.toml['last file dialog dir'] if 'last file dialog dir' in self.toml else os.getcwd()
		return folder if os.path.exists(folder) else ""

	def set_last_file_dialog_dir(self, folder):
		self.toml['last file dialog dir'] = folder
		self.dump()

	def project_dict(self) -> dict:
		if 'projects' not in self.toml:
			self.toml['projects'] = []
		return self.toml['projects']

	def project_id_stack(self) -> list:
		if 'project id stack' not in self.toml:
			self.toml['project id stack'] = []
		return self.toml['project id stack']

	def get_project_id(self, project_file) -> int:
		return self.project_dict()[project_file]

	def recent_list(self) -> list:
		if 'recent' not in self.toml:
			self.toml['recent'] = []
		return self.toml['recent']

	def is_filepath_relative_to_existing_project(self, filepath):
		folder = os.path.dirname(filepath)
		for project_filepath in self.project_dict():
			project_folder = os.path.dirname(project_filepath)
			if Path(folder).is_relative_to(project_folder) or Path(project_folder).is_relative_to(folder):
				return True
		return False

	def push_to_top_of_recent(self, project_filepath):
		if project_filepath in self.recent_list():
			self.recent_list().remove(project_filepath)
		self.recent_list().insert(0, project_filepath)
		self.dump()

	def is_valid_project_file(self, project_filepath):
		return project_filepath in self.project_dict()

	def get_recent_project_filepaths(self):
		return self.recent_list()

	def get_project_file(self, project_folder, project_name):
		return posixpath.join(project_folder, f"{project_name}.oly")

	def get_project_folder(self, project_file):
		return posixpath.dirname(project_file)

	def get_src_folder(self, project_folder):
		return posixpath.join(project_folder, "src/") if project_folder != "" else ""

	def get_res_folder(self, project_folder):
		return posixpath.join(project_folder, "res/") if project_folder != "" else ""

	def get_gen_folder(self, project_folder):
		return posixpath.join(project_folder, ".gen/") if project_folder != "" else ""

	def create_project(self, project_filepath):
		project_folder = self.get_project_folder(project_filepath)
		project_name = Path(project_filepath).stem
		Path(project_folder).mkdir(parents=True, exist_ok=True)
		src_folder = self.get_src_folder(project_folder)
		Path(src_folder).mkdir(parents=True, exist_ok=True)
		res_folder = self.get_res_folder(project_folder)
		Path(res_folder).mkdir(parents=True, exist_ok=True)
		gen_folder = self.get_gen_folder(project_folder)
		Path(gen_folder).mkdir(parents=True, exist_ok=True)

		with open(project_filepath, 'w') as f:
			f.write(self.get_default_project_context(project_folder, project_name))

		with open(posixpath.join(src_folder, "ProjectContext.h"), 'w') as f:
			f.write(self.project_context_h)

		with open(posixpath.join(src_folder, "ProjectContext.cpp"), 'w') as f:
			f.write(self.project_context_cpp)

		# TODO CMakeLists.txt, LICENSE

		if len(self.project_id_stack()) == 0:
			if 'project next id' not in self.toml:
				self.toml['project next id'] = 1
			project_id = self.toml['project next id']
			self.toml['project next id'] = project_id + 1
		else:
			project_id = self.project_id_stack().pop()

		self.project_dict()[project_filepath] = project_id
		self.dump()

	def remove_project_id(self, project_id):
		self.project_id_stack().append(project_id)
		send2trash.send2trash(f"projects/{project_id}")

	def delete_project(self, project_filepath):
		if project_filepath in self.project_dict():
			project_id = self.project_dict()[project_filepath]
			self.remove_project_id(project_id)
			del self.project_dict()[project_filepath]
			self.recent_list().remove(project_filepath)
			self.dump()
		# TODO v3 make a note in documentation that files are not deleted - they are simply removed from editor manifest, so as it to prevent unsafe folder deletion.

	def remove_nonexistent_projects(self):
		keep = []
		removed = []
		for project_filepath, project_id in self.project_dict().items():
			if os.path.exists(project_filepath):
				keep.append(project_filepath)
			else:
				self.remove_project_id(project_id)
				removed.append(project_filepath)
				self.recent_list().remove(project_filepath)
		self.toml['projects'] = keep
		self.dump()
		return removed


MANIFEST = ManifestTOML()
