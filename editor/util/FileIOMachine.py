import os
import string
from pathlib import Path
import random

import send2trash

from editor.util import ProjectContext


class FileIOMachine:
	def __init__(self):
		self.undo_stack = ProjectContext.UNDO_STACK

	@staticmethod
	def _trash_folder() -> Path:  # TODO v3 add .trash to project-specific .gitignore
		return ProjectContext.project_root_folder().joinpath(".trash")

	@staticmethod
	def _send_to_trash(path):
		send2trash.send2trash(Path(path).resolve())

	@staticmethod
	def _generate_hash_container():
		h = ''.join(random.choices(string.ascii_lowercase + string.digits, k=8))
		if os.path.exists(FileIOMachine._trash_folder().joinpath(h)):
			return FileIOMachine._generate_hash_container()
		else:
			return h

	def remove(self, path):
		path = Path(path)
		relative_path = path.relative_to(ProjectContext.project_root_folder())
		hash_container = self._generate_hash_container()
		#  TODO v3 use hash_container in undo command
		trash_path = self._trash_folder().joinpath(hash_container).joinpath(relative_path)
		os.makedirs(os.path.dirname(trash_path))
		os.rename(path, trash_path)


FIOMachine = FileIOMachine()
