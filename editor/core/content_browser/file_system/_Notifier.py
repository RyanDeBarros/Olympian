import os
from pathlib import Path

from ._InternalOps import _InternalOps


class _Notifier:
	from editor.core.ProjectContext import ProjectContext
	def __init__(self, project_context: ProjectContext, ops: _InternalOps):
		self.project_context = project_context
		self.content_browser = self.project_context.main_window.content_browser
		self.ops = ops

	def notify_browser_add_path(self, path: Path):
		if self.content_browser.should_display_path(path):
			self.content_browser.add_path(path)

	def notify_browser_add_paths(self, paths: list[Path]):
		paths = [path for path in paths if self.content_browser.should_display_path(path)]
		if len(paths) > 0:
			self.content_browser.add_paths(paths)

	def notify_browser_remove_path(self, path: Path):
		if self.content_browser.should_display_path(path):
			self.content_browser.remove_path(path)

	def notify_browser_remove_paths(self, paths: list[Path]):
		paths = [path for path in paths if self.content_browser.should_display_path(path)]
		if len(paths) > 0:
			self.content_browser.remove_paths(paths)

	def notify_main_tab_remove_path(self, path: Path):
		if path.is_dir():
			for dirpath, dirnames, filenames in os.walk(path):
				parent_path = Path(dirpath)
				for folder in dirnames:
					self.ops.main_tab_remove_path(parent_path / folder)
				for file in filenames:
					self.ops.main_tab_remove_path(parent_path / file)
		else:
			self.ops.main_tab_remove_path(path)

	def notify_main_tab_remove_paths(self, paths: list[Path]):
		for path in paths:
			self.notify_main_tab_remove_path(path)

	def notify_main_tab_rename_path(self, old_path: Path, new_path: Path):
		if old_path.is_dir():
			for dirpath, dirnames, filenames in os.walk(old_path):
				old_parent_path = Path(dirpath)
				new_parent_path = new_path / old_parent_path.relative_to(old_path)
				for folder in dirnames:
					self.ops.main_tab_rename_path(old_parent_path / folder, new_parent_path / folder)
				for file in filenames:
					self.ops.main_tab_rename_path(old_parent_path / file, new_parent_path / file)
		else:
			self.ops.main_tab_rename_path(old_path, new_path)
