from pathlib import Path
from typing import List, Optional

from PySide6.QtCore import QSize, QModelIndex
from PySide6.QtGui import QStandardItemModel, QStandardItem, QIcon, Qt
from PySide6.QtWidgets import QWidget, QFileDialog, QAbstractItemView, QListView

from editor import ui
from editor.util import ProjectContext


class PathItem:
	def __init__(self, name, is_file):
		self.name = name
		self.is_file = is_file


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
		self.path_items = {}

	def add_item(self, pi: PathItem):
		item = QStandardItem()
		item.setText(pi.name)
		# TODO v3 detect file type and use different icons
		item.setIcon(QIcon("res/images/file.svg" if pi.is_file else "res/images/folder.svg"))
		item.setTextAlignment(Qt.AlignmentFlag.AlignHCenter)
		self.model.appendRow(item)
		self.path_items[item.index()] = pi

	def add_items(self, items: List[PathItem]):
		for pi in items:
			self.add_item(pi)

	def clear_items(self):
		self.model.clear()

	def item_renamed(self, item: QStandardItem):
		pi = self.path_items[item.index()]
		if pi.name != item.text():
			if self.content_browser.rename_relative_file(pi.name, item.text()):
				self.path_items[item.index()].name = item.text()
			else:
				item.setText(pi.name)

	def item_double_clicked(self, index: QModelIndex):
		pi = self.path_items[index]
		if not pi.is_file:
			item = self.model.itemFromIndex(index)
			self.content_browser.open_relative_folder(item.text())


class ContentBrowser(QWidget):
	def __init__(self):
		super().__init__()
		self.win = None
		self.ui = ui.ContentBrowser.Ui_ContentBrowser()
		self.ui.setupUi(self)

		self.folder_view = self.ui.CBFolderView
		self.folder_view.content_browser = self

		self.res_folder = ProjectContext.project_resource_folder()

		self.ui.newAsset.clicked.connect(self.new_asset)
		self.ui.browseFolder.clicked.connect(self.browse_folder)

		self.current_folder = self.res_folder
		self.last_file_dialog_dir = self.current_folder
		self.open_folder(self.current_folder)

	def browse_folder(self):
		folder = QFileDialog.getExistingDirectory(self, "Select Folder", self.last_file_dialog_dir)
		if folder:
			self.last_file_dialog_dir = folder
			self.open_folder(folder)

	def new_asset(self):
		pass  # TODO

	def open_folder(self, folder):
		resolved_folder = Path(folder).resolve()
		if not resolved_folder.is_relative_to(self.res_folder):
			return

		self.current_folder = folder
		rel_folder = resolved_folder.relative_to(self.res_folder)
		self.ui.folderLineEdit.setText("RES://" + (rel_folder.as_posix() if str(rel_folder) != "." else ""))
		self.populate()

	def open_relative_folder(self, folder):
		self.open_folder(Path(self.current_folder).joinpath(folder))

	def rename_relative_file(self, old_name, new_name):
		old_name = Path(self.current_folder).joinpath(old_name)
		new_name = Path(self.current_folder).joinpath(new_name)
		# TODO
		return False

	def populate(self):
		self.folder_view.clear_items()
		items = [PathItem("..", False)]
		for path in Path(self.current_folder).iterdir():
			if path.is_dir():
				items.append(PathItem(path.name, False))
		for path in Path(self.current_folder).iterdir():
			if path.is_file():
				items.append(PathItem(path.name, True))
		self.folder_view.add_items(items)
