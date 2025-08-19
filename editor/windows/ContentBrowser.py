import os
import platform
import subprocess
from enum import Enum, auto
from pathlib import Path
from typing import List, Optional

from PySide6.QtCore import QSize, QModelIndex
from PySide6.QtGui import QStandardItemModel, QStandardItem, QIcon, Qt, QAction
from PySide6.QtWidgets import QWidget, QFileDialog, QAbstractItemView, QListView, QMenu, QMessageBox

from editor import ui
from editor.util import ProjectContext, FileIO


def alert_error(parent, title, desc):
	msg = QMessageBox(parent)
	msg.setIcon(QMessageBox.Icon.Warning)
	msg.setWindowTitle(title)
	msg.setText(desc)
	msg.exec()


class FileType(Enum):
	UNKNOWN = auto()
	DIRECTORY = auto()
	TEXT = auto()


class PathItem:
	def __init__(self, name, ftype: FileType):
		self.name = name
		self.ftype = ftype


class ContentBrowserFolderView(QListView):
	def __init__(self, parent=None):
		super().__init__(parent)
		self.content_browser: Optional[ContentBrowser] = None

		self.setViewMode(QListView.ViewMode.IconMode)
		self.setSelectionMode(QAbstractItemView.SelectionMode.ExtendedSelection)
		self.setResizeMode(QListView.ResizeMode.Adjust)
		self.setMovement(QListView.Movement.Static)
		self.setSpacing(10)

		self.icon_size = 64
		self.cell_width = 80
		self.cell_height = 100
		self.setIconSize(QSize(self.icon_size, self.icon_size))
		self.setGridSize(QSize(self.cell_width, self.cell_height))

		self.model = QStandardItemModel()
		self.setModel(self.model)
		self.model.itemChanged.connect(self.item_renamed)
		self.setEditTriggers(QAbstractItemView.EditTrigger.EditKeyPressed)
		self.path_items: dict[QModelIndex, PathItem] = {}

		self.setContextMenuPolicy(Qt.ContextMenuPolicy.CustomContextMenu)
		self.customContextMenuRequested.connect(self.show_context_menu)

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

		sorted_items = sorted(self.path_items.values(), key=sort_key)
		self.clear_items()
		self.add_items(sorted_items, sort=False)

	def add_item(self, pi: PathItem, editing=False, sort=True):
		item = QStandardItem()
		item.setText(pi.name)
		item.setIcon(QIcon(self.item_icon_path(pi.ftype)))
		item.setTextAlignment(Qt.AlignmentFlag.AlignHCenter)
		self.model.appendRow(item)
		self.path_items[item.index()] = pi

		if sort:
			self.sort()

		if editing:
			if sort:
				index = next((sorted_index for sorted_index, sorted_item in self.path_items.items() if sorted_item == pi), None)
			else:
				index = self.model.indexFromItem(item)
			self.edit(index)

	def add_items(self, items: List[PathItem], sort=True):
		for pi in items:
			self.add_item(pi, sort=False)
		if sort:
			self.sort()

	def clear_items(self):
		self.model.clear()
		self.path_items.clear()

	@staticmethod
	def item_icon_path(ftype: FileType):
		match ftype:
			case FileType.DIRECTORY:
				return "res/images/folder.svg"
			case FileType.TEXT:
				return "res/images/file.svg"

	def item_renamed(self, item: QStandardItem):
		pi = self.path_items[item.index()]
		if pi.name != item.text():
			if self.content_browser.rename_relative_file(pi.name, item.text()):
				self.path_items[item.index()].name = item.text()
				self.sort()
			else:
				item.setText(pi.name)

	# TODO v3 do something similar that only allows left click to select items
	def mouseDoubleClickEvent(self, event):
		if event.button() == Qt.MouseButton.LeftButton:
			index = self.indexAt(event.pos())
			if index.isValid():
				self.content_browser.open_item(self.path_items[index])
			else:
				event.ignore()
		else:
			event.ignore()

	def fill_no_item_context_menu(self, menu: QMenu):
		new_asset_menu = QMenu("New Asset", menu)
		new_asset_menu.setIcon(QIcon("res/images/file.svg"))

		txt = QAction(QIcon("res/images/file.svg"), "Text File", new_asset_menu)
		txt.triggered.connect(self.content_browser.new_text_file)
		new_asset_menu.addAction(txt)

		menu.addMenu(new_asset_menu)

		new_folder = QAction(QIcon("res/images/file.svg"), "New Folder", menu)
		new_folder.triggered.connect(self.content_browser.new_folder)
		menu.addAction(new_folder)

		refresh = QAction(QIcon("res/images/file.svg"), "Refresh", menu)
		refresh.triggered.connect(self.refresh_view)
		menu.addAction(refresh)

	def fill_single_item_context_menu(self, menu: QMenu, index: QModelIndex):
		rename = QAction(QIcon("res/images/file.svg"), "Rename", menu)
		rename.triggered.connect(lambda: self.edit(index))
		menu.addAction(rename)

		delete = QAction(QIcon("res/images/file.svg"), "Delete", menu)
		delete.triggered.connect(lambda: self.delete_item(index))
		menu.addAction(delete)

	def fill_multi_item_context_menu(self, menu: QMenu):
		delete = QAction(QIcon("res/images/file.svg"), "Delete", menu)
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
	def delete_item(self, index: QModelIndex):
		if not index.isValid():
			return False

		pi = self.path_items.get(index)
		if pi is None:
			return False

		# TODO v3 in all File IO operations, provide 'flush_to_disk' boolean parameter that determines whether changes should be applied in OS
		try:
			FileIO.move_to_trash(Path(self.content_browser.current_folder).joinpath(pi.name))
			del self.path_items[index]
			item = self.model.itemFromIndex(index)
			self.model.removeRow(item.row())
			return True
		except Exception as e:
			alert_error(self.content_browser, "Error - cannot delete item", str(e))
			return False

	def delete_selected_items(self):
		while self.selectedIndexes():
			index = self.selectedIndexes()[0]
			if not self.delete_item(index):
				break

	def refresh_view(self):
		self.clear_items()
		self.content_browser.populate()


# TODO v3 add optional tree view to the left of CBFolderView
class ContentBrowser(QWidget):
	def __init__(self):
		super().__init__()
		self.win = None
		self.ui = ui.ContentBrowser.Ui_ContentBrowser()
		self.ui.setupUi(self)

		self.folder_view = self.ui.CBFolderView
		self.folder_view.content_browser = self

		self.res_folder = ProjectContext.project_resource_folder()

		self.ui.browseFolder.clicked.connect(self.browse_folder)
		self.ui.openInExplorer.clicked.connect(self.open_in_explorer)

		self.current_folder = self.res_folder
		self.last_file_dialog_dir = self.current_folder
		self.open_folder(self.current_folder)

	def browse_folder(self):
		folder = QFileDialog.getExistingDirectory(self, "Select Folder", self.last_file_dialog_dir)
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
		if not resolved_folder.is_relative_to(self.res_folder):
			return

		self.current_folder = folder
		rel_folder = resolved_folder.relative_to(self.res_folder)
		self.ui.folderLineEdit.setText("RES://" + (rel_folder.as_posix() if str(rel_folder) != "." else ""))
		self.populate()

	def rename_relative_file(self, old_name, new_name):
		old_name = Path(self.current_folder).joinpath(old_name)
		new_name = Path(self.current_folder).joinpath(new_name)
		if old_name.exists() and not new_name.exists():
			try:
				os.rename(old_name, new_name)
				return True
			except OSError as e:
				alert_error(self, "Error - cannot rename item", str(e))
				return False
		return False

	def populate(self):
		self.folder_view.clear_items()
		items = [PathItem(name="..", ftype=FileType.DIRECTORY)]
		for path in Path(self.current_folder).iterdir():
			if path.is_dir():
				items.append(PathItem(name=path.name, ftype=FileType.DIRECTORY))
			elif path.is_file():
				# TODO v3 peek file to get type. With TOML, must load entire file, but with custom format, can peek to N characters.
				if path.suffix == ".txt":
					items.append(PathItem(name=path.name, ftype=FileType.TEXT))
		self.folder_view.add_items(items)

	def new_folder(self):
		folder_name = "NewFolder"
		i = 1
		while os.path.exists(os.path.join(self.current_folder, folder_name)):
			folder_name = f"NewFolder ({i})"
			i = i + 1
		folder_path = os.path.join(self.current_folder, folder_name)
		os.makedirs(folder_path)
		self.folder_view.add_item(PathItem(name=folder_name, ftype=FileType.DIRECTORY), editing=True)

	def new_text_file(self):
		pass  # TODO

	def open_item(self, pi: PathItem):
		match pi.ftype:
			case FileType.DIRECTORY:
				self.open_relative_folder(pi.name)
			case FileType.TEXT:
				self.open_text_file(pi.name)

	def open_relative_folder(self, folder):
		self.open_folder(Path(self.current_folder).joinpath(folder))

	def open_text_file(self, filepath):
		pass  # TODO
