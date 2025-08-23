import os
import platform
import subprocess
from enum import Enum, auto
from pathlib import Path
from typing import List, Optional

from PySide6.QtCore import QSize, QModelIndex, QEvent
from PySide6.QtGui import QStandardItemModel, QStandardItem, QIcon, Qt, QAction, QPixmap, QCursor
from PySide6.QtWidgets import QWidget, QFileDialog, QAbstractItemView, QListView, QMenu, QMessageBox, QToolTip

from editor.core import MainWindow, PREFERENCES
from editor.util import FileIOMachine


def alert_error(parent, title, desc):
	msg = QMessageBox(parent)
	msg.setIcon(QMessageBox.Icon.Warning)
	msg.setWindowTitle(title)
	msg.setText(desc)
	msg.exec()


class FileType(Enum):
	UNKNOWN = auto()
	DIRECTORY = auto()
	FILE = auto()
	TEXTURE = auto()
	FONT = auto()
	SIGNAL = auto()


class PathItem:
	def __init__(self, parent_folder: Path, name: str, ftype: FileType):
		self.full_path = parent_folder.resolve().joinpath(name)
		self.name = name
		self.ftype = ftype

	def icon(self, size: QSize):
		match self.ftype:
			case FileType.DIRECTORY:
				return QIcon(QPixmap("res/images/Folder.png").scaled(size))
			case FileType.FILE:
				return QIcon(QPixmap("res/images/File.png").scaled(size))
			case _:
				raise RuntimeError(f"icon_path(): unsupported file type {self.ftype}")


class ContentBrowserFolderView(QListView):
	def __init__(self, parent=None):
		super().__init__(parent)
		self.content_browser: Optional[ContentBrowser] = None

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
		self.path_items: list[PathItem] = []

		self.setContextMenuPolicy(Qt.ContextMenuPolicy.CustomContextMenu)
		self.customContextMenuRequested.connect(self.show_context_menu)

		self.file_machine: Optional[FileIOMachine] = None

	def init(self, content_browser):
		self.content_browser = content_browser
		self.file_machine = self.content_browser.win.project_context.file_machine

	def event(self, event):
		if event.type() == QEvent.Type.ToolTip:
			index = self.indexAt(event.pos())
			if index.isValid():
				pi = self.path_items[index.row()]
				if pi:
					QToolTip.showText(QCursor.pos(), str(pi.full_path), self)
					return True
		return super().event(event)

	def keyPressEvent(self, event):
		if event.key() == Qt.Key.Key_Delete:
			self.delete_selected_items()
		else:
			super().keyPressEvent(event)

	def sort(self):
		def sort_key(item: PathItem):
			if item.name == "..":
				return 0,
			elif item.ftype == FileType.DIRECTORY:
				return 1, item.name.lower()
			else:
				return 2, item.name.lower()

		sorted_items = sorted(self.path_items, key=sort_key)
		self.clear_items()
		self.add_items(sorted_items, sort=False)

	def add_item(self, pi: PathItem, editing=False, sort=True):
		item = QStandardItem()
		item.setText(pi.name)
		item.setIcon(pi.icon(self.iconSize()))
		item.setTextAlignment(Qt.AlignmentFlag.AlignHCenter)
		self.model.appendRow(item)
		self.path_items.insert(item.row(), pi)

		if sort:
			self.sort()

		if editing:
			if sort:
				index = self.path_items.index(pi)
			else:
				index = item.row()
			self.edit(self.model.index(index, 0))

	def add_items(self, items: List[PathItem], sort=True):
		for pi in items:
			self.add_item(pi, sort=False)
		if sort:
			self.sort()

	def clear_items(self):
		self.model.clear()
		self.path_items.clear()

	def item_renamed(self, item: QStandardItem, **kwargs):
		flush_to_disk = kwargs.get('flush_to_disk', True)
		pi = self.path_items[item.row()]
		if pi.name != item.text():
			if flush_to_disk:
				old_name = Path(self.content_browser.current_folder).joinpath(pi.name)
				new_name = Path(self.content_browser.current_folder).joinpath(item.text())
				if old_name.exists() and not new_name.exists():
					try:
						self.file_machine.rename(old_name, new_name)
					except OSError as e:
						alert_error(self, "Error - cannot rename item", str(e))
						item.setText(pi.name)
						return

			self.path_items[item.row()].name = item.text()
			self.sort()

	# TODO v3 do something similar that only allows left click to select items
	def mouseDoubleClickEvent(self, event):
		if event.button() == Qt.MouseButton.LeftButton:
			index = self.indexAt(event.pos())
			if index.isValid():
				self.content_browser.open_item(self.path_items[index.row()])
			else:
				event.ignore()
		else:
			event.ignore()

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

		refresh = QAction(QIcon("res/images/Refresh.png"), "Refresh", menu)
		refresh.triggered.connect(self.refresh_view)
		menu.addAction(refresh)

	def fill_single_item_context_menu(self, menu: QMenu, index: QModelIndex):
		rename = QAction(QIcon("res/images/Rename.png"), "Rename", menu)
		rename.triggered.connect(lambda: self.edit(index))
		menu.addAction(rename)

		delete = QAction(QIcon("res/images/Delete.png"), "Delete", menu)
		delete.triggered.connect(lambda: self.delete_item(index))
		menu.addAction(delete)

	def fill_multi_item_context_menu(self, menu: QMenu):
		delete = QAction(QIcon("res/images/Delete.png"), "Delete", menu)
		delete.triggered.connect(lambda: self.delete_selected_items())
		menu.addAction(delete)

	# TODO v3 custom icons for actions
	# TODO v3 copy/cut/paste options (these need to carry over when navigating through files
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

	# TODO v3 delete import file as well -> should be a method in PathItem
	def delete_item(self, index: QModelIndex, **kwargs):
		flush_to_disk = kwargs.get('flush_to_disk', True)
		follow_through = True
		if PREFERENCES.prompt_user_when_deleting_paths:
			reply = QMessageBox.question(self, f"Confirm Action", f"Are you sure you want to delete the selected item?",
										 QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No, QMessageBox.StandardButton.No)
			follow_through = reply == QMessageBox.StandardButton.Yes
		if follow_through:
			assert index.row() < len(self.path_items)
			pi = self.path_items.pop(index.row())
			self.model.removeRow(index.row())
			if flush_to_disk:
				self.file_machine.remove(self.content_browser.current_folder.joinpath(pi.name))

	# TODO v3 delete import files as well -> should be a method in PathItem
	def delete_selected_items(self, **kwargs):
		flush_to_disk = kwargs.get('flush_to_disk', True)
		if self.selectedIndexes():
			follow_through = True
			if PREFERENCES.prompt_user_when_deleting_paths:
				reply = QMessageBox.question(self, f"Confirm Action", f"Are you sure you want to delete the selected item(s)?",
											 QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No, QMessageBox.StandardButton.No)
				follow_through = reply == QMessageBox.StandardButton.Yes
			if follow_through:
				indexes = self.selectedIndexes()
				indexes.sort(key=lambda index: index.row(), reverse=True)
				remove_paths = []
				for index in indexes:
					assert index.row() < len(self.path_items)
					pi = self.path_items.pop(index.row())
					remove_paths.append(self.content_browser.current_folder.joinpath(pi.name))

				while self.selectedIndexes():
					index = self.selectedIndexes()[0]
					self.model.removeRow(index.row())

				if flush_to_disk:
					self.file_machine.remove_together(remove_paths)

	def refresh_view(self):
		self.clear_items()
		self.content_browser.populate()


# TODO v3 add optional tree view to the left of CBFolderView
class ContentBrowser(QWidget):
	def __init__(self):
		super().__init__()
		from editor import ui
		self.ui = ui.ContentBrowser.Ui_ContentBrowser()
		self.ui.setupUi(self)
		self.win: Optional[MainWindow] = None

		self.folder_view = self.ui.CBFolderView

		self.ui.browseFolder.clicked.connect(self.browse_folder)
		self.ui.openInExplorer.clicked.connect(self.open_in_explorer)

		self.current_folder: Optional[Path] = None
		self.last_file_dialog_dir: Optional[Path] = None

	def init(self, win: MainWindow):
		self.win = win
		self.current_folder = self.win.project_context.res_folder
		self.last_file_dialog_dir = self.current_folder
		self.open_folder(self.current_folder)

		self.folder_view.init(self)

	def browse_folder(self):
		folder = QFileDialog.getExistingDirectory(self, "Select Folder", str(self.last_file_dialog_dir))
		if folder:
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

	def open_folder(self, folder):
		resolved_folder = Path(folder).resolve()
		if not resolved_folder.is_relative_to(self.win.project_context.res_folder):
			return

		self.current_folder = folder
		rel_folder = resolved_folder.relative_to(self.win.project_context.res_folder)
		self.ui.folderLineEdit.setText("RES://" + (rel_folder.as_posix() if str(rel_folder) != "." else ""))
		self.populate()

	def populate(self):
		self.folder_view.clear_items()
		items = [PathItem(parent_folder=self.current_folder, name="..", ftype=FileType.DIRECTORY)]
		for path in Path(self.current_folder).iterdir():
			if path.is_dir():
				items.append(PathItem(parent_folder=self.current_folder, name=path.name, ftype=FileType.DIRECTORY))
			elif path.is_file():
				# TODO v3 peek file to get type. With TOML, must load entire file, but with custom format, can peek to N characters.
				if path.suffix == ".oly":
					pass  # TODO v3 add if asset and not import
				else:
					# TODO v3 check for existing import file
					items.append(PathItem(parent_folder=self.current_folder, name=path.name, ftype=FileType.FILE))
		self.folder_view.add_items(items)

	def new_folder(self, **kwargs):
		flush_to_disk = kwargs.get('flush_to_disk', True)
		folder_name = "NewFolder"
		i = 1
		while os.path.exists(os.path.join(self.current_folder, folder_name)):
			folder_name = f"NewFolder ({i})"
			i = i + 1
		folder_path = os.path.join(self.current_folder, folder_name)
		if flush_to_disk:
			self.folder_view.file_machine.new_folder(folder_path)
		self.folder_view.add_item(PathItem(parent_folder=self.current_folder, name=folder_name, ftype=FileType.DIRECTORY), editing=True)

	def new_file(self):
		pass  # TODO v3

	def new_signal_asset(self):
		pass  # TODO v3

	def open_item(self, pi: PathItem):
		match pi.ftype:
			case FileType.DIRECTORY:
				self.open_relative_folder(pi.name)
			case FileType.FILE:
				self.open_file(pi.name)

	def open_relative_folder(self, folder):
		self.open_folder(self.current_folder.joinpath(folder))

	def open_file(self, filepath):
		pass  # TODO v3

	def open_signal_asset(self, filepath):
		pass  # TODO v3
