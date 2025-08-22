import os
import string
from pathlib import Path
import random

import send2trash

from editor.core import ProjectContext


class FileIOMachine:
	def __init__(self, project_context: ProjectContext):
		self.project_context = project_context
		self.undo_stack = self.project_context.undo_stack

	@staticmethod
	def _send_to_trash(path):
		send2trash.send2trash(Path(path).resolve())

	def _trash_folder(self) -> Path:  # TODO v3 add .trash to project-specific .gitignore
		return self.project_context.project_folder.joinpath(".trash")

	def _generate_hash_container(self):
		h = ''.join(random.choices(string.ascii_lowercase + string.digits, k=8))
		if os.path.exists(self._trash_folder().joinpath(h)):
			return self._generate_hash_container()
		else:
			return h

	def remove(self, path):
		path = Path(path)
		relative_path = path.relative_to(self.project_context.project_folder)
		hash_container = self._generate_hash_container()
		#  TODO v3 use hash_container in undo command
		trash_path = self._trash_folder().joinpath(hash_container).joinpath(relative_path)
		os.makedirs(os.path.dirname(trash_path))
		os.rename(path, trash_path)
