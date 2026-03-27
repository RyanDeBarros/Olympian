from abc import ABC, abstractmethod
from pathlib import Path
from typing import Optional

from editor.core import FileSystemWatcher
from editor.core.buffers import BufferPath
from editor.tools import eprint


class AbstractBuffer(FileSystemWatcher, ABC):
	def __init__(self, buf: BufferPath):
		super().__init__()
		self.buf = buf
		# TODO v7.2 make sure to close all buffers on editor exit - but cache list of opened buffers. Then open them all on editor start

	def __init_subclass__(cls, **kwargs):
		super().__init_subclass__(**kwargs)
		if cls is not AbstractBuffer and not cls.__abstractmethods__:
			BufferChooser.instance().classes.append(cls)

	@classmethod
	@abstractmethod
	def matches_asset(cls, asset_path: Path) -> bool:
		eprint(f"{cls.__name__}: Not Implemented")
		return False

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
			if self.__class__.matches_asset(dest):
				self.buf.on_asset_moved(dest)
			else:
				eprint(f"{self.__class__.__name__}: Not Implemented")  # TODO v7.2 handle different asset type - also, self.close() here might not work with the asset file moved

	def open(self) -> None:
		if not self.buf.buffer_path.exists():
			self.buf.buffer_path.touch()
			self.on_open()

	def on_open(self) -> None:
		pass

	def close(self) -> None:
		if self.buf.buffer_path.exists() and self.on_close():
			self.buf.buffer_path.unlink()

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
		for cls in self.classes:
			if cls.matches_asset(asset_path):
				return cls(BufferPath.from_asset(asset_path))
		else:
			return None
