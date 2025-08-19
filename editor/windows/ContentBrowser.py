import os
import platform
import subprocess
from enum import Enum, auto
from pathlib import Path
from typing import List, Optional

from PySide6.QtCore import QSize, QModelIndex
from PySide6.QtGui import QStandardItemModel, QStandardItem, QIcon, Qt, QAction
from PySide6.QtWidgets import QWidget, QFileDialog, QAbstractItemView, QListView, QMenu

from editor import ui
from editor.util import ProjectContext


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
		self.doubleClicked.connect(self.item_double_clicked)
		self.setEditTriggers(QAbstractItemView.EditTrigger.EditKeyPressed)
		self.path_items: dict[QModelIndex, PathItem] = {}

		self.setContextMenuPolicy(Qt.ContextMenuPolicy.CustomContextMenu)
		self.customContextMenuRequested.connect(self.show_context_menu)

	def add_item(self, pi: PathItem):
		item = QStandardItem()
		item.setText(pi.name)
		item.setIcon(QIcon(self.item_icon_path(pi.ftype)))
		item.setTextAlignment(Qt.AlignmentFlag.AlignHCenter)
		self.model.appendRow(item)
		self.path_items[item.index()] = pi

	def add_items(self, items: List[PathItem]):
		for pi in items:
			self.add_item(pi)

	def clear_items(self):
		self.model.clear()

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
			else:
				item.setText(pi.name)

	def item_double_clicked(self, index: QModelIndex):
		self.content_browser.open_item(self.path_items[index])

	def show_context_menu(self, pos):
		index = self.indexAt(pos)
		menu = QMenu(self)

		# TODO v3 custom icons for actions
		if index.isValid():
			rename = QAction(QIcon("res/images/file.svg"), "Rename")
			rename.triggered.connect(lambda: self.edit(index))
			menu.addAction(rename)

			delete = QAction(QIcon("res/images/file.svg"), "Delete")
			delete.triggered.connect(lambda: self.delete_item(index))
			menu.addAction(delete)
		else:
			new_asset_menu = QMenu("New Asset")
			new_asset_menu.setIcon(QIcon("res/images/file.svg"))

			txt = QAction(QIcon("res/images/file.svg"), "Text File")
			txt.triggered.connect(self.content_browser.new_text_file)
			new_asset_menu.addAction(txt)

			menu.addMenu(new_asset_menu)

			new_folder = QAction(QIcon("res/images/file.svg"), "New Folder")
			new_folder.triggered.connect(self.content_browser.new_folder)
			menu.addAction(new_folder)

		menu.exec(self.viewport().mapToGlobal(pos))

	def delete_item(self, index: QModelIndex):
		pass  # TODO


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
		# TODO
		return False

	def populate(self):
		self.folder_view.clear_items()
		items = [PathItem("..", FileType.DIRECTORY)]
		for path in Path(self.current_folder).iterdir():
			if path.is_dir():
				items.append(PathItem(path.name, FileType.DIRECTORY))
		for path in Path(self.current_folder).iterdir():
			if path.is_file():
				# TODO v3 peek file to get type. With TOML, must load entire file, but with custom format, can peek to N characters.
				items.append(PathItem(path.name, FileType.TEXT))
		self.folder_view.add_items(items)

	def new_folder(self):
		pass  # TODO

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
