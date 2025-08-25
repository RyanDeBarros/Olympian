from __future__ import annotations

from abc import ABC, abstractmethod
from pathlib import Path

from PySide6.QtCore import QSize
from typing import TYPE_CHECKING

if TYPE_CHECKING:
	from editor.core.ContentBrowser import ContentBrowser


class AbstractPathItem(ABC):
	def __init__(self, parent_folder: Path, name: str):
		self.full_path = parent_folder.resolve().joinpath(name)

	def __eq__(self, other):
		return type(self) is type(other) and self.__dict__ == other.__dict__

	@abstractmethod
	def icon(self, size: QSize):
		pass

	@abstractmethod
	def ui_name(self):
		pass

	@abstractmethod
	def renamed_filepath(self, name: str):
		pass

	def sorting_key(self):
		return 2, self.ui_name().lower()

	def rename_to(self, name: str):
		self.full_path = self.renamed_filepath(name)

	@abstractmethod
	def new_item(self, browser: ContentBrowser):
		pass

	@abstractmethod
	def open(self, browser: ContentBrowser):
		pass
