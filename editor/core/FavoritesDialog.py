import bisect
from pathlib import Path

from PySide6.QtGui import QIcon
from PySide6.QtWidgets import QDialog, QLineEdit, QPushButton, QHeaderView, QAbstractScrollArea

from editor import ui, TOMLAdapter
from .ContentBrowser import ContentBrowser
from editor.core import block_signals
from editor.core.common import Alerts


class FavoritesDialog(QDialog):
	def __init__(self, browser: ContentBrowser):
		super().__init__(browser)
		self.browser = browser
		self.project_context = self.browser.win.project_context
		self.friendly_path = self.project_context.to_friendly_resource_path(self.browser.current_folder)
		self.toml_content = TOMLAdapter.load(self.project_context.favorites_file)
		if 'favorites' not in self.toml_content:
			self.toml_content['favorites'] = []
		self.favorites: list[str] = self.toml_content['favorites']

		self.ui = ui.ContentBrowserFavorites.Ui_Favorites()
		self.ui.setupUi(self)

		self.setWindowTitle("Favorites")
		self.setWindowIcon(QIcon("res/images/Olympian.png"))

		self.ui.favoritesTable.horizontalHeader().setSectionResizeMode(0, QHeaderView.ResizeMode.Stretch)
		self.ui.favoritesTable.horizontalHeader().setSectionResizeMode(1, QHeaderView.ResizeMode.Fixed)
		self.ui.favoritesTable.horizontalHeader().setSectionResizeMode(2, QHeaderView.ResizeMode.Fixed)
		self.ui.favoritesTable.setSizeAdjustPolicy(QAbstractScrollArea.SizeAdjustPolicy.AdjustToContents)
		self.ui.favoritesTable.setColumnWidth(1, 50)
		self.ui.favoritesTable.setColumnWidth(2, 20)

		for i in range(len(self.favorites)):
			self.add_favorite_row_to_table(i)

		self.ui.currentFolder.setText(self.friendly_path)
		self.ui.favoriteCheckbox.setChecked(self.friendly_path in self.favorites)
		self.ui.favoriteCheckbox.checkStateChanged.connect(self.toggle_current_folder_favorite)
		self.ui.pruneInvalidFavorites.clicked.connect(self.prune_invalid_favorites)
		self.ui.clearFavorites.clicked.connect(self.clear_favorites)

	def dump(self):
		TOMLAdapter.dump(self.project_context.favorites_file, self.toml_content, {'type': 'res_favorites'})

	def open_favorite_callback(self, index):
		def callback():
			favorite_path = self.project_context.from_friendly_resource_path(self.favorites[index])
			if favorite_path.exists():
				self.close()
				self.browser.open_folder(favorite_path)
			else:
				Alerts.alert_error(self, "Path does not exist", "Press the Prune Invalid Favorites to remove non-existent paths.")
		return callback

	def delete_favorite_impl(self, index, expect_favorite_checked: bool = True):
		assert index >= 0

		if self.friendly_path in self.favorites and index == self.favorites.index(self.friendly_path):
			with block_signals(self.ui.favoriteCheckbox) as favoriteCheckbox:
				assert not expect_favorite_checked or favoriteCheckbox.isChecked()
				favoriteCheckbox.setChecked(False)

		self.favorites.pop(index)
		self.dump()

		self.ui.favoritesTable.removeRow(index)
		for i in range(index, self.ui.favoritesTable.rowCount()):
			open_favorite = self.ui.favoritesTable.cellWidget(i, 1)
			assert isinstance(open_favorite, QPushButton)
			open_favorite.clicked.disconnect()
			open_favorite.clicked.connect(self.open_favorite_callback(i))

			delete_favorite = self.ui.favoritesTable.cellWidget(i, 2)
			assert isinstance(delete_favorite, QPushButton)
			delete_favorite.clicked.disconnect()
			delete_favorite.clicked.connect(self.delete_favorite_callback(i))

	def delete_favorite_callback(self, index):
		def callback():
			self.delete_favorite_impl(index)
		return callback

	def add_favorite_row_to_table(self, index):
		self.ui.favoritesTable.insertRow(index)

		favorite_filepath = QLineEdit(self.favorites[index], readOnly=True)
		self.ui.favoritesTable.setCellWidget(index, 0, favorite_filepath)

		open_favorite = QPushButton("Open")
		open_favorite.clicked.connect(self.open_favorite_callback(index))
		self.ui.favoritesTable.setCellWidget(index, 1, open_favorite)

		delete_favorite = QPushButton(QIcon("res/images/Delete.png"), "")
		delete_favorite.clicked.connect(self.delete_favorite_callback(index))
		self.ui.favoritesTable.setCellWidget(index, 2, delete_favorite)

	def toggle_current_folder_favorite(self):
		if self.ui.favoriteCheckbox.isChecked():
			assert self.friendly_path not in self.favorites

			def key_of(friendly_path):
				return Path(friendly_path[len(self.project_context.res_friendly_prefix):]).parts
			index = bisect.bisect_left([key_of(favorite) for favorite in self.favorites], key_of(self.friendly_path))

			self.favorites.insert(index, self.friendly_path)
			self.add_favorite_row_to_table(index)
			self.dump()
		else:
			self.delete_favorite_impl(self.favorites.index(self.friendly_path), False)

	def prune_invalid_favorites(self):
		index = 0
		while index < len(self.favorites):
			favorite_filepath = self.ui.favoritesTable.cellWidget(index, 0)
			assert isinstance(favorite_filepath, QLineEdit)
			actual_filepath = self.project_context.from_friendly_resource_path(favorite_filepath)

			if actual_filepath.exists() and actual_filepath.is_dir():
				index += 1
				continue

			self.delete_favorite_impl(index)

	def clear_favorites(self):
		self.favorites.clear()
		self.dump()

		while self.ui.favoritesTable.rowCount() > 0:
			self.ui.favoritesTable.removeRow(0)

		with block_signals(self.ui.favoriteCheckbox) as favoriteCheckbox:
			favoriteCheckbox.setChecked(False)
