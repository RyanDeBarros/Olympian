from __future__ import annotations

import bisect
import os
import platform
import subprocess
from enum import IntEnum
from pathlib import Path
from typing import List, Optional

from PySide6.QtCore import QSize, QModelIndex, QEvent, QItemSelectionModel, QItemSelection
from PySide6.QtGui import QStandardItemModel, QStandardItem, QIcon, Qt, QAction, QCursor, QShortcut, \
	QKeySequence, QUndoStack, QUndoCommand
from PySide6.QtWidgets import QWidget, QFileDialog, QAbstractItemView, QListView, QMenu, QMessageBox, QToolTip, QDialog, \
	QVBoxLayout, QUndoView, QCheckBox

from editor.core import MainWindow
from editor.core.EditorPreferences import PREFERENCES
from editor.core.FileIOMachine import FileIOMachine
from editor.core.common import Alerts
from editor.core.path_items import *


class ContentBrowserFolderView(QListView):
	def __init__(self, parent=None):
		super().__init__(parent)
		self.content_browser: Optional[ContentBrowser] = None
		self.file_machine: Optional[FileIOMachine] = None
		self.clipboard_paths: list[Path] = []

		self.setViewMode(QListView.ViewMode.IconMode)
		self.setSelectionMode(QAbstractItemView.SelectionMode.ExtendedSelection)
		self.setResizeMode(QListView.ResizeMode.Adjust)
		self.setMovement(QListView.Movement.Static)
		self.setSpacing(10)
		self.setIconSize(QSize(64, 64))
		self.setGridSize(QSize(80, 100))

		self.model = QStandardItemModel()
		self.setModel(self.model)
		self.model.itemChanged.connect(self.item_renamed)
		self.setEditTriggers(QAbstractItemView.EditTrigger.EditKeyPressed)
		self.path_items: list[AbstractPathItem] = []

		self.setContextMenuPolicy(Qt.ContextMenuPolicy.CustomContextMenu)
		self.customContextMenuRequested.connect(self.show_context_menu)
		select_all_shortcut = QShortcut(QKeySequence("Ctrl+A"), self)
		select_all_shortcut.activated.connect(self.selectAll)

	def init(self, content_browser):
		self.content_browser = content_browser
		self.file_machine = self.content_browser.file_machine

	def event(self, event):
		if event.type() == QEvent.Type.ToolTip:
			index = self.indexAt(event.pos())
			if index.isValid():
				pi = self.path_items[index.row()]
				if pi:
					QToolTip.showText(QCursor.pos(), pi.full_path.as_posix(), self)
					return True
		return super().event(event)

	def keyPressEvent(self, event):
		if event.key() == Qt.Key.Key_Delete:
			self.delete_selected_items()
		elif event.key() == Qt.Key.Key_Enter or event.key() == Qt.Key.Key_Return:
			if self.state() != QAbstractItemView.State.EditingState:
				for index in self.selectedIndexes():
					self.path_items[index.row()].open(self.content_browser)
			else:
				super().keyPressEvent(event)
		else:
			super().keyPressEvent(event)

	def mousePressEvent(self, event):
		if event.button() == Qt.MouseButton.LeftButton:
			super().mousePressEvent(event)
		elif event.button() == Qt.MouseButton.RightButton:
			index = self.indexAt(event.pos())
			if index.isValid():
				if index not in self.selectedIndexes():
					self.selectionModel().clearSelection()
				self.selectionModel().select(index, QItemSelectionModel.SelectionFlag.Select)
				super().mousePressEvent(event)
			else:
				self.selectionModel().clearSelection()
		else:
			event.ignore()

	def mouseDoubleClickEvent(self, event):
		if event.button() == Qt.MouseButton.LeftButton:
			index = self.indexAt(event.pos())
			if index.isValid():
				self.path_items[index.row()].open(self.content_browser)
			else:
				event.ignore()
		else:
			event.ignore()

	def selectionCommand(self, index, event=None):
		if index.row() == 0:
			return QItemSelectionModel.SelectionFlag.NoUpdate
		return super().selectionCommand(index, event)

	def selectionChanged(self, selected: QItemSelection, deselected: QItemSelection):
		super().selectionChanged(selected, deselected)

		index0 = self.model.index(0, 0)
		if self.selectionModel().isSelected(index0):
			self.selectionModel().select(index0, QItemSelectionModel.SelectionFlag.Deselect)

	def sorted_item_index(self, item: AbstractPathItem):
		return bisect.bisect_left([item.sorting_key() for item in self.path_items], item.sorting_key())

	def add_item(self, pi: AbstractPathItem, editing=False):
		if pi in self.path_items:
			return

		index = self.sorted_item_index(pi)
		self.path_items.insert(index, pi)

		item = QStandardItem()
		item.setText(pi.ui_name())
		item.setIcon(pi.icon(self.iconSize()))
		item.setTextAlignment(Qt.AlignmentFlag.AlignHCenter)
		self.model.insertRow(index, item)

		if editing:
			self.edit(self.model.index(index, 0))

	def add_items(self, items: List[AbstractPathItem]):
		for item in items:
			self.add_item(item)

	def remove_item(self, item: AbstractPathItem):
		if item not in self.path_items:
			return
		index = self.path_items.index(item)
		self.model.removeRow(index)
		self.path_items.pop(index)

	def remove_items(self, items: list[AbstractPathItem]):
		for item in items:
			self.remove_item(item)

	def clear_items(self):
		self.model.clear()
		self.path_items.clear()

	def item_renamed(self, item: QStandardItem):
		pi = self.path_items[item.row()]
		if pi.ui_name() != item.text():
			old_name = pi.full_path
			new_name = pi.renamed_filepath(item.text())
			if old_name.exists() and not new_name.exists():
				try:
					self.file_machine.rename(old_name, new_name, False)
				except OSError as e:
					Alerts.alert_error(self, "Error - cannot rename item", str(e))
					item.setText(pi.ui_name())

	def fill_no_item_context_menu(self, menu: QMenu):
		new_file = QAction(QIcon("res/images/File.png"), "New File", menu)
		new_file.triggered.connect(self.content_browser.new_file)
		menu.addAction(new_file)

		new_asset_menu = QMenu("New Asset", menu)
		new_asset_menu.setIcon(QIcon("res/images/NewAsset.png"))

		signal = QAction(QIcon("res/images/InputSignal.png"), "Input Signal", new_asset_menu)
		signal.triggered.connect(self.content_browser.new_signal_asset)
		new_asset_menu.addAction(signal)

		menu.addMenu(new_asset_menu)

		new_folder = QAction(QIcon("res/images/Folder.png"), "New Folder", menu)
		new_folder.triggered.connect(self.content_browser.new_folder)
		menu.addAction(new_folder)

		menu.addSeparator()

		paste = QAction(QIcon("res/images/Paste.png"), "Paste", menu)
		paste.triggered.connect(lambda: self.paste_items())
		menu.addAction(paste)

		menu.addSeparator()

		refresh = QAction(QIcon("res/images/Refresh.png"), "Refresh", menu)
		refresh.triggered.connect(self.refresh_view)
		menu.addAction(refresh)

	def fill_single_item_context_menu(self, menu: QMenu, index: QModelIndex):
		if index.row() == 0:
			return

		copy = QAction(QIcon("res/images/Copy.png"), "Copy", menu)
		copy.triggered.connect(lambda: self.copy_items([index]))
		menu.addAction(copy)

		move = QAction(QIcon("res/images/Move.png"), "Move", menu)
		move.triggered.connect(lambda: self.move_items([index]))
		menu.addAction(move)

		menu.addSeparator()

		reimport = QAction(QIcon("res/images/Import.png"), "Import", menu)
		reimport.triggered.connect(lambda: self.import_items([index]))
		menu.addAction(reimport)

		menu.addSeparator()

		rename = QAction(QIcon("res/images/Rename.png"), "Rename", menu)
		rename.triggered.connect(lambda: self.edit(index))
		menu.addAction(rename)

		delete = QAction(QIcon("res/images/Delete.png"), "Delete", menu)
		delete.triggered.connect(lambda: self.delete_item(index))
		menu.addAction(delete)

	def fill_multi_item_context_menu(self, menu: QMenu):
		copy = QAction(QIcon("res/images/Copy.png"), "Copy", menu)
		copy.triggered.connect(lambda: self.copy_items(self.selectedIndexes()))
		menu.addAction(copy)

		move = QAction(QIcon("res/images/Move.png"), "Move", menu)
		move.triggered.connect(lambda: self.move_items(self.selectedIndexes()))
		menu.addAction(move)

		menu.addSeparator()

		reimport = QAction(QIcon("res/images/Import.png"), "Import", menu)
		reimport.triggered.connect(lambda: self.import_items(self.selectedIndexes()))
		menu.addAction(reimport)

		menu.addSeparator()

		delete = QAction(QIcon("res/images/Delete.png"), "Delete", menu)
		delete.triggered.connect(lambda: self.delete_selected_items())
		menu.addAction(delete)

	def show_context_menu(self, pos):
		menu = QMenu(self)

		selected_indexes = self.selectedIndexes()
		if len(selected_indexes) == 0:
			index = self.indexAt(pos)
			if index.isValid():
				self.fill_single_item_context_menu(menu, index)
			else:
				self.fill_no_item_context_menu(menu)
		elif len(selected_indexes) == 1:
			self.fill_single_item_context_menu(menu, selected_indexes[0])
		else:
			self.fill_multi_item_context_menu(menu)

		menu.exec(self.viewport().mapToGlobal(pos))

	def delete_item(self, index: QModelIndex):
		follow_through = True
		if PREFERENCES.prompt_user_when_deleting_paths:
			reply = QMessageBox.question(self, f"Confirm Action", f"Are you sure you want to delete the selected item?",
										 QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No, QMessageBox.StandardButton.No)
			follow_through = reply == QMessageBox.StandardButton.Yes
		if follow_through:
			assert index.row() < len(self.path_items)
			pi = self.path_items.pop(index.row())
			self.model.removeRow(index.row())
			self.file_machine.remove(pi.full_path)

	def delete_selected_items(self):
		if self.selectedIndexes():
			follow_through = True
			if PREFERENCES.prompt_user_when_deleting_paths:
				reply = QMessageBox.question(self, f"Confirm Action", f"Are you sure you want to delete the selected item(s)?",
											 QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No, QMessageBox.StandardButton.No)
				follow_through = reply == QMessageBox.StandardButton.Yes
			if follow_through:
				indexes = [index.row() for index in self.selectedIndexes()]
				indexes.sort(reverse=True)
				remove_paths = []
				for index in indexes:
					assert index < len(self.path_items)
					pi = self.path_items.pop(index)
					remove_paths.append(pi.full_path)

				while self.selectedIndexes():
					index = self.selectedIndexes()[0]
					self.model.removeRow(index.row())

				self.file_machine.remove_together(remove_paths)

	def import_items(self, indexes: list[QModelIndex]):
		for index in indexes:
			self.path_items[index.row()].on_import(self.content_browser)

	@staticmethod
	def get_next_available_file(path: Path, existing_paths: list[Path]):
		index = 0
		base_file = path
		for _ in path.suffixes:
			base_file = base_file.with_suffix('')
		extension = ''.join(path.suffixes)
		renamed_path = path
		while renamed_path.exists() or renamed_path in existing_paths:
			index += 1
			renamed_path = Path(f"{base_file} ({index}){extension}")
		return renamed_path

	@staticmethod
	def get_next_available_folder(path: Path, existing_paths: list[Path]):
		index = 0
		renamed_path = path
		while renamed_path.exists() or renamed_path in existing_paths:
			index += 1
			renamed_path = Path(f"{path} ({index})")
		return renamed_path

	class _DuplicatePathAction(IntEnum):
		REPLACE = 0
		RENAME = 1
		CANCEL_INDIVIDUAL = 2

	@staticmethod
	def _prompt_duplicate_path_action(duplicate_path: Path, action_name: str) -> tuple[_DuplicatePathAction, bool] | None:
		alert = QMessageBox(QMessageBox.Icon.Warning, f"Path already exists", f"Path {duplicate_path} already exists.\nHow do you want to handle the {action_name}?")
		rename = alert.addButton(f"Rename with (*) counter", QMessageBox.ButtonRole.ActionRole)
		replace = alert.addButton(f"Replace existing", QMessageBox.ButtonRole.ActionRole)
		cancel_individual = alert.addButton(f"Cancel individual {action_name}", QMessageBox.ButtonRole.RejectRole)
		alert.addButton(f"Cancel all {action_name}s", QMessageBox.ButtonRole.RejectRole)
		checkbox = QCheckBox("Do selected action for all items")
		alert.setCheckBox(checkbox)
		alert.exec()
		if alert.clickedButton() == replace:
			action = ContentBrowserFolderView._DuplicatePathAction.REPLACE
		elif alert.clickedButton() == rename:
			action = ContentBrowserFolderView._DuplicatePathAction.RENAME
		elif alert.clickedButton() == cancel_individual:
			action = ContentBrowserFolderView._DuplicatePathAction.CANCEL_INDIVIDUAL
		else:
			return None
		return action, checkbox.isChecked()

	class _PathTransferData:
		def __init__(self, from_path: Path, to_path: Path, additional_existing_paths: list[Path]):
			self.from_path = from_path
			self.to_path = to_path
			self.additional_existing_paths = additional_existing_paths
			self.can_transfer = True
			self.replace_duplicate = False
			assert from_path.exists()

		def will_duplicate(self):
			return self.to_path.exists() or self.to_path in self.additional_existing_paths

		def handle_duplicate(self, action: ContentBrowserFolderView._DuplicatePathAction):
			match action:
				case ContentBrowserFolderView._DuplicatePathAction.REPLACE:
					self.replace_duplicate = True
				case ContentBrowserFolderView._DuplicatePathAction.RENAME:
					if self.from_path.is_file():
						self.to_path = ContentBrowserFolderView.get_next_available_file(self.to_path, self.additional_existing_paths)
					else:
						self.to_path = ContentBrowserFolderView.get_next_available_folder(self.to_path, self.additional_existing_paths)
				case ContentBrowserFolderView._DuplicatePathAction.CANCEL_INDIVIDUAL:
					self.can_transfer = False

		def can_append(self):
			return self.can_transfer and not (self.replace_duplicate and self.from_path == self.to_path)

	class _MultiPathTransferData:
		def __init__(self, to_folder: Path, action_name: str):
			self.to_folder = to_folder
			self.action_name = action_name
			self.from_paths: list[Path] = []
			self.to_paths: list[Path] = []
			self.replace_existing: list[bool] = []
			self.repeated_action: Optional[ContentBrowserFolderView._DuplicatePathAction] = None

		def _process(self, from_path: Path):
			transfer_data = ContentBrowserFolderView._PathTransferData(from_path=from_path, to_path=self.to_folder.joinpath(from_path.name),
																	   additional_existing_paths=self.to_paths)
			if transfer_data.will_duplicate():
				if self.repeated_action is not None:
					action = self.repeated_action
				else:
					reply = ContentBrowserFolderView._prompt_duplicate_path_action(transfer_data.to_path, self.action_name)
					if reply is None:
						return
					action, repeat = reply
					if repeat:
						self.repeated_action = action
				transfer_data.handle_duplicate(action)

			if transfer_data.can_append():
				self.from_paths.append(transfer_data.from_path)
				self.to_paths.append(transfer_data.to_path)
				self.replace_existing.append(transfer_data.replace_duplicate)

		def process_transfer(self, paths: list[Path]):
			for path in paths:
				self._process(from_path=path)

		def can_transfer(self):
			return len(self.from_paths) > 0


	def move_items(self, indexes: list[QModelIndex]):
		folder = QFileDialog.getExistingDirectory(self, "Move To", self.content_browser.current_folder.as_posix())
		if folder:
			folder = Path(folder).resolve()
			if folder and folder.is_relative_to(self.content_browser.win.project_context.res_folder) and folder != self.content_browser.current_folder:
				multi_move_data = ContentBrowserFolderView._MultiPathTransferData(to_folder=folder, action_name="move")
				multi_move_data.process_transfer([self.path_items[index.row()].full_path for index in indexes])
				if multi_move_data.can_transfer():
					try:
						self.file_machine.rename_all(multi_move_data.from_paths, multi_move_data.to_paths, multi_move_data.replace_existing)
					except OSError as e:
						Alerts.alert_error(self, "Error - cannot complete move", str(e))

	def copy_items(self, indexes: list[QModelIndex]):
		self.clipboard_paths = [self.path_items[index.row()].full_path for index in indexes]

	def paste_items(self):
		multi_paste_data = ContentBrowserFolderView._MultiPathTransferData(to_folder=self.content_browser.current_folder, action_name="paste")
		multi_paste_data.process_transfer([clipboard_path for clipboard_path in self.clipboard_paths if clipboard_path.exists()])
		if multi_paste_data.can_transfer():
			try:
				self.file_machine.copy_paste(multi_paste_data.from_paths, multi_paste_data.to_paths, multi_paste_data.replace_existing)
			except OSError as e:
				Alerts.alert_error(self, "Error - cannot complete paste", str(e))

	def refresh_view(self):
		current_folder = self.content_browser.current_folder
		while not current_folder.exists():
			if current_folder.is_relative_to(self.content_browser.win.project_context.res_folder):
				current_folder = current_folder.parent
			else:
				Alerts.alert_error(self, 'Error - cannot load project filesystem', 'Likely cause is external modification of filesystem. Force closing editor...')
				self.content_browser.win.quit_project(1)

		if current_folder != self.content_browser.current_folder:
			self.content_browser.current_folder = current_folder
			friendly_folder = self.content_browser.win.project_context.to_friendly_resource_path(current_folder)
			self.content_browser.ui.folderLineEdit.setText(friendly_folder)
		self.content_browser.win.project_context.refresh()
		self.content_browser.populate()


class ContentBrowser(QWidget):
	def __init__(self):
		super().__init__()
		from editor import ui
		self.ui = ui.ContentBrowser.Ui_ContentBrowser()
		self.ui.setupUi(self)
		self.win: Optional[MainWindow] = None

		self.folder_view = self.ui.CBFolderView
		self.undo_stack = QUndoStack()
		self.file_machine: Optional[FileIOMachine] = None

		self.ui.browseFolder.clicked.connect(self.browse_folder)
		self.ui.openInExplorer.clicked.connect(self.open_in_explorer)

		self.ui.undoButton.setIcon(QIcon("res/images/Undo.png"))
		self.ui.undoButton.clicked.connect(self.undo_stack.undo)
		undo_shortcut = QShortcut(QKeySequence("Ctrl+Z"), self)
		undo_shortcut.activated.connect(self.undo_in_context)

		self.ui.redoButton.setIcon(QIcon("res/images/Redo.png"))
		self.ui.redoButton.clicked.connect(self.undo_stack.redo)
		redo_shortcut = QShortcut(QKeySequence("Ctrl+Shift+Z"), self)
		redo_shortcut.activated.connect(self.redo_in_context)

		move_shortcut = QShortcut(QKeySequence("Ctrl+M"), self)
		move_shortcut.activated.connect(self.move_in_context)

		copy_shortcut = QShortcut(QKeySequence("Ctrl+C"), self)
		copy_shortcut.activated.connect(self.copy_in_context)

		paste_shortcut = QShortcut(QKeySequence("Ctrl+V"), self)
		paste_shortcut.activated.connect(self.paste_in_context)

		history_menu = QMenu(self.ui.historyToolButton)
		history_show = QAction("Show History", history_menu)
		history_show.triggered.connect(self.show_undo_stack)
		history_menu.addAction(history_show)
		history_clear = QAction("Clear History", history_menu)
		history_clear.triggered.connect(self.clear_undo_stack)
		history_menu.addAction(history_clear)
		trash_clear = QAction("Clear History + Trash", history_menu)
		trash_clear.triggered.connect(self.clear_history_and_trash)
		history_menu.addAction(trash_clear)
		self.ui.historyToolButton.setMenu(history_menu)
		self.ui.historyToolButton.clicked.connect(lambda: self.ui.historyToolButton.showMenu())

		from .FavoritesDialog import FavoritesDialog
		self.ui.openFavorites.clicked.connect(lambda: FavoritesDialog(self).exec())
		favorites_shortcut = QShortcut(QKeySequence("Shift+Alt+F"), self)
		favorites_shortcut.activated.connect(lambda: FavoritesDialog(self).exec() if self.underMouse() else None)

		self.current_folder: Optional[Path] = None
		self.last_file_dialog_dir: Optional[Path] = None

	def init(self, win: MainWindow):
		self.win = win
		self.current_folder = self.win.project_context.res_folder
		self.last_file_dialog_dir = self.current_folder
		self.open_folder(self.current_folder, add_to_history=False)

		self.file_machine = FileIOMachine(self.win.project_context)
		self.folder_view.init(self)

	def show_undo_stack(self):
		dialog = QDialog(self)
		dialog.setWindowTitle("Undo Stack")
		layout = QVBoxLayout(dialog)
		layout.addWidget(QUndoView(self.undo_stack))
		dialog.exec()

	def clear_undo_stack(self):
		reply = QMessageBox.question(self, "Confirm history clear", "Are you sure you want to delete the content browser's undo history? This action cannot be undone.",
									 QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No, QMessageBox.StandardButton.No)
		if reply == QMessageBox.StandardButton.Yes:
			self.undo_stack.clear()

	def clear_history_and_trash(self):
		reply = QMessageBox.question(self, "Confirm history + trash clear", "Are you sure you want to delete the content browser's undo history and the recycling bin? This action cannot be undone.",
									 QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No, QMessageBox.StandardButton.No)
		if reply == QMessageBox.StandardButton.Yes:
			self.undo_stack.clear()
			self.file_machine.clear_trash()

	def undo_in_context(self):
		if self.underMouse():
			self.undo_stack.undo()

	def redo_in_context(self):
		if self.underMouse():
			self.undo_stack.redo()

	def move_in_context(self):
		if self.underMouse():
			self.folder_view.move_items(self.folder_view.selectedIndexes())

	def copy_in_context(self):
		if self.underMouse():
			self.folder_view.copy_items(self.folder_view.selectedIndexes())

	def paste_in_context(self):
		if self.underMouse():
			self.folder_view.paste_items()

	def browse_folder(self):
		folder = QFileDialog.getExistingDirectory(self, "Select Folder", self.last_file_dialog_dir.as_posix())
		if folder:
			folder = Path(folder).resolve()
			if folder and folder.is_relative_to(self.win.project_context.res_folder):
				self.last_file_dialog_dir = folder
				self.open_folder(folder)

	def open_in_explorer(self):
		system = platform.system()
		if system == "Windows":
			os.startfile(os.path.normpath(self.current_folder))
		elif system == "Darwin":
			subprocess.run(["open", self.current_folder])
		else:
			subprocess.run(["xdg-open", self.current_folder])

	def open_folder(self, folder, add_to_history: bool = True):
		folder = Path(folder).resolve()
		if not folder.is_relative_to(self.win.project_context.res_folder):
			return

		class UCOpenFolder(QUndoCommand):
			def __init__(self, browser: ContentBrowser, old_folder: Path, new_folder: Path):
				super().__init__("Navigate Browser")
				self.browser = browser
				self.old_folder = old_folder
				self.new_folder = new_folder

			def open(self, folder):
				self.browser.current_folder = folder
				friendly_folder = self.browser.win.project_context.to_friendly_resource_path(folder)
				self.browser.ui.folderLineEdit.setText(friendly_folder)
				self.browser.populate()

			def undo(self):
				self.open(self.old_folder)

			def redo(self):
				self.open(self.new_folder)

		cmd = UCOpenFolder(self, self.current_folder, folder)
		if self.current_folder != folder and add_to_history:
			self.undo_stack.push(cmd)
		else:
			cmd.redo()

	def should_display_path(self, path: Path):
		return path.parent == self.current_folder

	def populate(self):
		self.folder_view.clear_items()
		items = [FolderPathItem(self.current_folder.joinpath(".."))]
		for path in Path(self.current_folder).iterdir():
			item = get_path_item(self.current_folder.joinpath(path))
			if item is not None:
				items.append(item)
		self.folder_view.add_items(items)

	def add_path(self, path: Path):
		assert path.resolve().parent == self.current_folder
		item = get_path_item(self.current_folder.joinpath(path))
		if item is not None:
			self.folder_view.add_item(item)

	def add_paths(self, paths: list[Path]):
		items = []
		for path in paths:
			assert path.resolve().parent == self.current_folder
			item = get_path_item(self.current_folder.joinpath(path))
			if item is not None:
				items.append(item)
		self.folder_view.add_items(items)

	def remove_path(self, path: Path):
		assert path.resolve().parent == self.current_folder
		item = get_path_item(self.current_folder.joinpath(path))
		if item is not None:
			self.folder_view.remove_item(item)

	def remove_paths(self, paths: list[Path]):
		items = []
		for path in paths:
			assert path.resolve().parent == self.current_folder
			item = get_path_item(self.current_folder.joinpath(path))
			if item is not None:
				items.append(item)
		self.folder_view.remove_items(items)

	def refresh_item(self, item: AbstractPathItem):
		if item.full_path.resolve().parent == self.current_folder:
			self.folder_view.remove_item(item)
			self.add_path(item.full_path)

	def new_folder(self):
		FolderPathItem.new_item(self)

	def new_file(self):
		StandardFilePathItem.new_item(self)

	def new_signal_asset(self):
		InputSignalPathItem.new_item(self)
