import os
import random
import string
from pathlib import Path

import send2trash

from ...path_items import get_path_item


class _InternalOps:
	from editor.core.ProjectContext import ProjectContext
	def __init__(self, project_context: ProjectContext):
		self.project_context = project_context
		self.content_browser = self.project_context.main_window.content_browser
		self.main_tab_holder = self.project_context.main_window.tab_holder

	@staticmethod
	def send_to_trash(path):
		send2trash.send2trash(Path(path).resolve())

	@staticmethod
	def move_to(old_path: Path, new_path: Path):
		assert not os.path.exists(new_path)
		os.makedirs(os.path.dirname(new_path), exist_ok=True)
		if not new_path.exists():
			os.rename(old_path, new_path)
		else:
			raise RuntimeError(f"File {new_path} already exists.")

	@staticmethod
	def rm_all_dirs(folder: Path):
		for root, dirs, _ in os.walk(folder, topdown=False):
			for d in dirs:
				os.rmdir(os.path.join(root, d))
		os.rmdir(folder)

	@staticmethod
	def resolve_all(paths: list[Path | str]):
		return [Path(path).resolve() for path in paths]

	@staticmethod
	def assert_same_length(*lists):
		lengths = [len(lst) for lst in lists]
		assert len(set(lengths)) == 1
		return lengths[0]

	def _generate_hash_container(self):
		h = ''.join(random.choices(string.ascii_lowercase + string.digits, k=8))
		if (self.project_context.trash_folder / h).exists():
			return self._generate_hash_container()
		else:
			return h

	def generate_hash_path(self):
		return self.project_context.trash_folder / self._generate_hash_container()

	def main_tab_remove_path(self, path: Path):
		uids = self.main_tab_holder.uids
		if path in uids:
			self.main_tab_holder.remove_tab(uids.index(path))

	def main_tab_rename_path(self, old_path: Path, new_path: Path):
		uids = self.main_tab_holder.uids
		if old_path in uids:
			assert new_path not in uids
			item = get_path_item(self.content_browser.current_folder / old_path)
			assert item is not None
			item.full_path = new_path
			index = uids.index(old_path)
			uids.pop(index)
			uids.insert(index, new_path)
			tab = self.main_tab_holder.editor_tab_at(index)
			tab.rename(item)
