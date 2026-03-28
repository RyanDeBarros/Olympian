from abc import ABC, abstractmethod
from pathlib import Path
from typing import Optional

from editor.core import FileSystemWatcher
from editor.tools import eprint, TOMLAdapter
from . import BufferPath
from .processing import Metadata, AssetType


class AbstractBuffer(FileSystemWatcher, ABC):
	def __init__(self, buf: BufferPath):
		super().__init__()
		self.buf = buf

	# TODO v7.2 make sure to close all buffers on editor exit - but cache list of opened buffers. Then open them all on editor start

	@classmethod
	@abstractmethod
	def matches_asset(cls, buf: BufferPath) -> bool:
		eprint(f"{cls.__name__}: Not Implemented")
		return False

	@staticmethod
	def simple_matches_asset(asset: BufferPath, asset_type: AssetType, file_extensions: list[str]) -> bool:
		if asset.is_imported():
			if asset.import_path().exists():
				return Metadata.asset_type(TOMLAdapter.meta(asset.import_path())) == asset_type
			else:
				return asset.asset_path.suffix in file_extensions
		else:
			return asset.asset_type() == asset_type

	def on_modified(self, path: Path, was_dir: bool) -> None:
		if path == self.buf.asset_path:
			self.on_asset_modified()

	def on_asset_modified(self) -> None:
		pass

	def on_deleted(self, path: Path, was_dir: bool) -> None:
		if path == self.buf.asset_path:
			self.on_asset_deleted()

	def on_asset_deleted(self) -> None:
		pass

	def on_moved(self, src: Path, dest: Path, was_dir: bool) -> None:
		if src == self.buf.asset_path:
			if self.__class__.matches_asset(BufferPath.from_asset(dest)):
				self.buf.on_asset_moved(dest)
			else:
				eprint(f"{self.__class__.__name__}: Not Implemented")  # TODO v7.2 handle different asset type - also, self.close() here might not work with the asset file moved

	def open(self) -> None:
		if not self.buf.buffer_path.exists():
			self.buf.buffer_path.parent.mkdir(parents=True)
			self.buf.buffer_path.touch()
			self.on_open()

	def on_open(self) -> None:
		pass

	def close(self) -> bool:
		if self.buf.buffer_path.exists() and self.on_close():
			self.buf.buffer_path.unlink()
			return True
		else:
			return False

	def on_close(self) -> bool:
		return True


class BufferChooser:
	_instance: Optional["BufferChooser"] = None

	def __init__(self):
		self.classes: list[type[AbstractBuffer]] = []

	@classmethod
	def instance(cls) -> "BufferChooser":
		if cls._instance is None:
			cls._instance = cls()
		return cls._instance

	def buffer(self, asset_path: Path) -> Optional[AbstractBuffer]:
		asset = BufferPath.from_asset(asset_path)
		for cls in self.classes:
			if cls.matches_asset(asset):
				return cls(asset)
		else:
			return None
