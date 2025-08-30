import os
import random
import shutil
import string
from pathlib import Path
from typing import Optional

import send2trash
from PySide6.QtGui import QUndoCommand

from editor.core.path_items import get_path_item


class FileIOMachine:
	from editor.core.ProjectContext import ProjectContext
	def __init__(self, project_context: ProjectContext):
		self.project_context = project_context
		self.content_browser = self.project_context.main_window.content_browser
		self.main_tab_holder = self.project_context.main_window.tab_holder
		self.undo_stack = self.content_browser.undo_stack

	@staticmethod
	def _send_to_trash(path):
		send2trash.send2trash(Path(path).resolve())

	def clear_trash(self):
		self._send_to_trash(self.project_context.trash_folder)
		os.makedirs(self.project_context.trash_folder)

	def _generate_hash_container(self):
		h = ''.join(random.choices(string.ascii_lowercase + string.digits, k=8))
		if os.path.exists(self.project_context.trash_folder.joinpath(h)):
			return self._generate_hash_container()
		else:
			return h

	def generate_hash_path(self):
		return self.project_context.trash_folder.joinpath(self._generate_hash_container())

	def remove(self, path: Path):
		self.undo_stack.push(UCDeletePaths(self, [Path(path).resolve()]))

	def remove_together(self, paths: list[Path]):
		self.undo_stack.push(UCDeletePaths(self, [Path(path).resolve() for path in paths]))

	def rename(self, old_path: Path, new_path: Path, replace_existing: bool):
		self.undo_stack.push(UCRenamePath(self, Path(old_path).resolve(), Path(new_path).resolve(), replace_existing))

	def rename_all(self, old_paths: list[Path], new_paths: list[Path], replace_existing: list[bool]):
		assert len(old_paths) == len(new_paths) and len(old_paths) == len(replace_existing)
		if len(old_paths) == 1:
			self.rename(old_paths[0], new_paths[0], replace_existing[0])
		else:
			self.undo_stack.push(UCRenameMultiplePaths(self, [Path(old_path).resolve() for old_path in old_paths],
													   [Path(new_path).resolve() for new_path in new_paths],
													   replace_existing))

	def copy_paste(self, copied_paths: list[Path], pasted_paths: list[Path], replace_existing: list[bool]):
		assert len(copied_paths) == len(pasted_paths) and len(copied_paths) == len(replace_existing)
		self.undo_stack.push(UCCopyPastePaths(self, [Path(copied_path).resolve() for copied_path in copied_paths],
											  [Path(pasted_path).resolve() for pasted_path in pasted_paths],
											  replace_existing))

	def new_folder(self, folder: Path):
		self.undo_stack.push(UCNewFolder(self, Path(folder).resolve()))

	def new_file(self, file: Path):
		self.undo_stack.push(UCNewFile(self, Path(file).resolve()))

	def uc_browser_add_path(self, path: Path):
		if self.content_browser.should_display_path(path):
			self.content_browser.add_path(path)

	def uc_browser_add_paths(self, paths: list[Path]):
		paths = [path for path in paths if self.content_browser.should_display_path(path)]
		if len(paths) > 0:
			self.content_browser.add_paths(paths)

	def uc_browser_remove_path(self, path: Path):
		if self.content_browser.should_display_path(path):
			self.content_browser.remove_path(path)

	def uc_browser_remove_paths(self, paths: list[Path]):
		paths = [path for path in paths if self.content_browser.should_display_path(path)]
		if len(paths) > 0:
			self.content_browser.remove_paths(paths)

	def _main_tab_remove_path(self, path: Path):
		uids = self.main_tab_holder.uids
		if path in uids:
			self.main_tab_holder.remove_tab(uids.index(path))

	def uc_main_tab_remove_path(self, path: Path):
		if path.is_dir():
			for dirpath, dirnames, filenames in os.walk(path):
				parent_path = Path(dirpath)
				for folder in dirnames:
					self._main_tab_remove_path(parent_path.joinpath(folder))
				for file in filenames:
					self._main_tab_remove_path(parent_path.joinpath(file))
		else:
			self._main_tab_remove_path(path)

	def uc_main_tab_remove_paths(self, paths: list[Path]):
		for path in paths:
			self.uc_main_tab_remove_path(path)

	def _main_tab_rename_path(self, old_path: Path, new_path: Path):
		uids = self.main_tab_holder.uids
		if old_path in uids:
			assert new_path not in uids
			item = get_path_item(self.content_browser.current_folder.joinpath(old_path))
			assert item is not None
			item.full_path = new_path
			index = uids.index(old_path)
			uids.pop(index)
			uids.insert(index, new_path)
			tab = self.main_tab_holder.editor_tab_at(index)
			tab.rename(item)

	def uc_main_tab_rename_path(self, old_path: Path, new_path: Path):
		if old_path.is_dir():
			for dirpath, dirnames, filenames in os.walk(old_path):
				old_parent_path = Path(dirpath)
				new_parent_path = new_path.joinpath(old_parent_path.relative_to(old_path))
				for folder in dirnames:
					self._main_tab_rename_path(old_parent_path.joinpath(folder), new_parent_path.joinpath(folder))
				for file in filenames:
					self._main_tab_rename_path(old_parent_path.joinpath(file), new_parent_path.joinpath(file))
		else:
			self._main_tab_rename_path(old_path, new_path)


def _move_to(old_path: Path, new_path: Path):
	assert not os.path.exists(new_path)
	os.makedirs(os.path.dirname(new_path), exist_ok=True)
	if not new_path.exists():
		os.rename(old_path, new_path)
	else:
		raise RuntimeError(f"File {new_path} already exists.")


def _rm_all_dirs(folder: Path):
	for root, dirs, _ in os.walk(folder, topdown=False):
		for d in dirs:
			os.rmdir(os.path.join(root, d))
	os.rmdir(folder)


class UCDeletePaths(QUndoCommand):
	def __init__(self, machine: FileIOMachine, paths: list[Path]):
		super().__init__("Delete Path(s)")
		self.machine = machine
		self.num_paths = len(paths)

		self.paths = paths
		self.trash_paths: Optional[list[Path]] = None
		self.hash_path: Optional[Path] = None

	def _generate_trash_paths(self):
		self.hash_path = self.machine.generate_hash_path()
		self.trash_paths = [self.hash_path.joinpath(path.relative_to(self.machine.project_context.project_folder)) for
							path in self.paths]

	def undo(self):
		for i in range(self.num_paths):
			_move_to(self.trash_paths[i], self.paths[i])
			import_path = Path(self.trash_paths[i].as_posix() + '.oly')
			if import_path.exists():
				_move_to(import_path, Path(self.paths[i].as_posix() + '.oly'))
		_rm_all_dirs(self.hash_path)
		self.trash_paths = None
		self.hash_path = None
		self.machine.uc_browser_add_paths(self.paths)

		for path in self.paths:
			item = get_path_item(path)
			assert item is not None
			item.on_new(self.machine.content_browser)

	def redo(self):
		self._generate_trash_paths()
		self.machine.uc_browser_remove_paths(self.paths)
		self.machine.uc_main_tab_remove_paths(self.paths)

		for path in self.paths:
			item = get_path_item(path)
			assert item is not None
			item.on_delete(self.machine.content_browser)

		for i in range(self.num_paths):
			_move_to(self.paths[i], self.trash_paths[i])
			import_path = Path(self.paths[i].as_posix() + '.oly')
			if import_path.exists():
				_move_to(import_path, Path(self.trash_paths[i].as_posix() + '.oly'))


class UCRenamePath(QUndoCommand):
	def __init__(self, machine: FileIOMachine, old_path: Path, new_path: Path, replace_existing: bool):
		super().__init__("Rename Path")
		self.machine = machine
		self.old_path = old_path
		self.new_path = new_path
		self.replacer = UCDeletePaths(self.machine, [self.new_path]) if replace_existing else None

	def undo(self):
		self.machine.uc_browser_remove_path(self.new_path)
		self.machine.uc_main_tab_rename_path(self.new_path, self.old_path)
		_move_to(self.new_path, self.old_path)
		import_path = Path(self.new_path.as_posix() + '.oly')
		if import_path.exists():
			_move_to(import_path, Path(self.old_path.as_posix() + '.oly'))
		self.machine.uc_browser_add_path(self.old_path)

		item = get_path_item(self.old_path)
		assert item is not None
		item.on_rename(self.machine.content_browser, self.new_path)

		if self.replacer is not None:
			self.replacer.undo()

	def redo(self):
		if self.replacer is not None:
			self.replacer.redo()

		self.machine.uc_browser_remove_path(self.old_path)
		self.machine.uc_main_tab_rename_path(self.old_path, self.new_path)
		_move_to(self.old_path, self.new_path)
		import_path = Path(self.old_path.as_posix() + '.oly')
		if import_path.exists():
			_move_to(import_path, Path(self.new_path.as_posix() + '.oly'))
		self.machine.uc_browser_add_path(self.new_path)

		item = get_path_item(self.new_path)
		assert item is not None
		item.on_rename(self.machine.content_browser, self.old_path)


class UCRenameMultiplePaths(QUndoCommand):
	def __init__(self, machine: FileIOMachine, old_paths: list[Path], new_paths: list[Path],
				 replace_existing: list[bool]):
		super().__init__("Rename Multiple Paths")
		self.machine = machine
		assert len(old_paths) == len(new_paths) and len(old_paths) == len(replace_existing)
		self.num_paths = len(old_paths)
		self.renamers = [UCRenamePath(self.machine, old_path, new_path, replace) for old_path, new_path, replace in
						 zip(old_paths, new_paths, replace_existing)]

	def undo(self):
		for i in range(self.num_paths):
			self.renamers[i].undo()

	def redo(self):
		for i in range(self.num_paths):
			self.renamers[i].redo()


class UCCopyPastePaths(QUndoCommand):
	def __init__(self, machine: FileIOMachine, copied_paths: list[Path], pasted_paths: list[Path],
				 replace_existing: list[bool]):
		super().__init__("Paste Copied Paths")
		self.machine = machine
		assert len(copied_paths) == len(pasted_paths) and len(copied_paths) == len(replace_existing)
		self.copied_paths = copied_paths
		self.pasted_paths = pasted_paths
		self.num_paths = len(self.copied_paths)
		self.replacers = [UCDeletePaths(self.machine, [pasted_path]) if replace else None for replace, pasted_path in
						 zip(replace_existing, self.pasted_paths)]
		self.first_paste = True
		self.paste_deleter = UCDeletePaths(self.machine, self.pasted_paths)

	def undo(self):
		self.paste_deleter.redo()
		for i in range(self.num_paths):
			if self.replacers[i] is not None:
				self.replacers[i].undo()

	def redo(self):
		for i in range(self.num_paths):
			if self.replacers[i] is not None:
				self.replacers[i].redo()

		if self.first_paste:
			for i in range(self.num_paths):
				if self.copied_paths[i].is_file():
					shutil.copy2(self.copied_paths[i], self.pasted_paths[i])
					import_file = Path(self.copied_paths[i].as_posix() + '.oly')
					if import_file.exists():
						shutil.copy2(import_file, Path(self.pasted_paths[i].as_posix() + '.oly'))
				elif self.copied_paths[i].is_dir():
					shutil.copytree(self.copied_paths[i], self.pasted_paths[i])
				else:
					assert False
		else:
			self.paste_deleter.undo()

		if self.first_paste:
			self.first_paste = False
			for i in range(self.num_paths):
				self.machine.uc_browser_add_path(self.pasted_paths[i])

				item = get_path_item(self.pasted_paths[i])
				assert item is not None
				item.on_new(self.machine.content_browser)


class UCNewFolder(QUndoCommand):
	def __init__(self, machine: FileIOMachine, folder: Path):
		super().__init__("New Folder")
		self.machine = machine
		self.folder = folder
		self.makedirs = True
		self.hash_path: Optional[Path] = None
		self.trash_path: Optional[Path] = None

	def _generate_trash_path(self):
		self.hash_path = self.machine.generate_hash_path()
		self.trash_path = self.hash_path.joinpath(self.folder.relative_to(self.machine.project_context.project_folder))

	def undo(self):
		item = get_path_item(self.folder)
		assert item is not None
		item.on_delete(self.machine.content_browser)

		_move_to(self.folder, self.trash_path)

		self._generate_trash_path()
		self.machine.uc_browser_remove_path(self.folder)

	def redo(self):
		if self.makedirs:
			self.makedirs = False
			os.makedirs(self.folder)
		else:
			_move_to(self.trash_path, self.folder)
			_rm_all_dirs(self.hash_path)
			self.trash_path = None
			self.hash_path = None
			self.machine.uc_browser_add_path(self.folder)

		item = get_path_item(self.folder)
		assert item is not None
		item.on_new(self.machine.content_browser)


class UCNewFile(QUndoCommand):
	def __init__(self, machine: FileIOMachine, file: Path):
		super().__init__("New File")
		self.machine = machine
		self.file = file
		self.touch = True
		self.hash_path: Optional[Path] = None
		self.trash_path: Optional[Path] = None

	def _generate_trash_path(self):
		self.hash_path = self.machine.generate_hash_path()
		self.trash_path = self.hash_path.joinpath(self.file.relative_to(self.machine.project_context.project_folder))

	def undo(self):
		self._generate_trash_path()
		self.machine.uc_browser_remove_path(self.file)
		self.machine.uc_main_tab_remove_path(self.file)

		item = get_path_item(self.file)
		assert item is not None
		item.on_delete(self.machine.content_browser)

		_move_to(self.file, self.trash_path)
		import_path = Path(self.file.as_posix() + '.oly')
		if import_path.exists():
			_move_to(import_path, Path(self.trash_path.as_posix() + '.oly'))

	def redo(self):
		if self.touch:
			self.touch = False
			self.file.touch()
		else:
			_move_to(self.trash_path, self.file)
			import_path = Path(self.trash_path.as_posix() + '.oly')
			if import_path.exists():
				_move_to(import_path, Path(self.file.as_posix() + '.oly'))
			_rm_all_dirs(self.hash_path)
			self.trash_path = None
			self.hash_path = None
			self.machine.uc_browser_add_path(self.file)

		item = get_path_item(self.file)
		assert item is not None
		item.on_new(self.machine.content_browser)
