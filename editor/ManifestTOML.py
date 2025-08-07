import os
from pathlib import Path
import posixpath

import toml


def default_context_toml(project_folder, project_name):
	return {
        "context": {
			"window_hint": {},
			"window": {
				"width": 1440,
				"height": 1080,
				"title": "Olympian Engine"
			},
			"gamepads": 1,
			"logger": {
				"logfile": os.path.join(project_folder, f"{project_name}.log"),
				"append": True,
				"console": True
			},
			"sprite_batch": {
				"initial sprites": 100,
				"new textures": 10
			},
			"polygon_batch": {
				"primitives": 0
			},
			"ellipse_batch": {
				"ellipses": 0
			},
			"text_batch": {
				"initial glyphs": 100,
				"new textures": 10
			}
		}
	}


def project_context_header():
	return """#pragma once

#include \"Olympian.h\"

namespace oly
{
    struct ProjectContext
    {
        ProjectContext();
        ProjectContext(const ProjectContext&) = delete;
        ProjectContext(ProjectContext&&) = delete;
        
    private:
        context::Context context;
    };
}
"""

def project_context_cpp(project_file, resource_folder):
	return f"""#include "ProjectContext.h"

namespace oly
{{
	ProjectContext::ProjectContext() : context(\"{project_file}\", \"{resource_folder}\") {{}}
}}
"""


class ManifestTOML:
	def __init__(self):
		self.filepath = 'data/manifest.toml'
		with open(self.filepath, 'r') as f:
			self.toml = toml.load(f)

	def dump(self):
		with open(self.filepath, 'w') as f:
			toml.dump(self.toml, f)

	def get_last_file_dialog_dir(self):
		folder = self.toml['last file dialog dir'] if 'last file dialog dir' in self.toml else os.getcwd()
		return folder if os.path.exists(folder) else ""

	def set_last_file_dialog_dir(self, folder):
		self.toml['last file dialog dir'] = folder
		self.dump()

	def project_list(self) -> list:
		if 'projects' not in self.toml:
			self.toml['projects'] = []
		return self.toml['projects']

	def recent_list(self) -> list:
		if 'recent' not in self.toml:
			self.toml['recent'] = []
		return self.toml['recent']

	def is_filepath_relative_to_existing_project(self, filepath):
		for project_filepath in self.project_list():
			if Path(filepath).is_relative_to(project_filepath) or Path(project_filepath).is_relative_to(filepath):
				return True
		return False

	def push_to_top_of_recent(self, project_filepath):
		if project_filepath in self.recent_list():
			self.recent_list().remove(project_filepath)
		self.recent_list().insert(0, project_filepath)
		self.dump()

	def is_valid_project_file(self, project_filepath):
		return project_filepath in self.project_list()

	def get_recent_project_filepaths(self):
		return self.project_list()

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
			toml.dump(default_context_toml(project_folder, project_name), f)

		with open(posixpath.join(src_folder, "ProjectContext.h"), 'w') as f:
			f.write(project_context_header())

		with open(posixpath.join(src_folder, "ProjectContext.cpp"), 'w') as f:
			f.write(project_context_cpp(project_filepath, res_folder))

		# TODO CMakeLists.txt, LICENSE

		self.project_list().append(project_filepath)
		self.dump()

	def delete_project(self, project_filepath):
		if project_filepath in self.project_list():
			self.project_list().remove(project_filepath)
			self.recent_list().remove(project_filepath)
			self.dump()
			# TODO make note that files are not deleted - they are simply removed from editor manifest, so as it to prevent unsafe folder deletion.


MANIFEST = ManifestTOML()
