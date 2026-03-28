from abc import ABC, abstractmethod
from pathlib import Path
from typing import Optional, Type, Callable

import toml

from editor.core import FileSystemWatcher
from editor.tools import eprint, TOMLAdapter
from . import BufferPath
from .processing import Metadata, AssetType, EnumUtils


class AbstractBuffer(FileSystemWatcher, ABC):
	def __init__(self, buf: BufferPath):
		super().__init__()
		self.buf = buf
		self.d = {}
		self.indent = 0

	# TODO v7.2 make sure to close all buffers on editor exit - but cache list of opened buffers. Then open them all on editor start

	@classmethod
	@abstractmethod
	def matches_asset(cls, buf: BufferPath) -> bool:
		eprint(f"{cls.__name__}: Not Implemented")
		return False

	@staticmethod
	def simple_matches_asset(asset: BufferPath, asset_type: AssetType, file_extensions: list[str], resource=True) -> bool:
		from editor.core import ProgramState
		if resource and not asset.asset_path.relative_to(ProgramState.instance().resource_dir()):
			return False

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
			self.buf.buffer_path.parent.mkdir(parents=True, exist_ok=True)
			self.buf.buffer_path.touch()
			self.d = toml.loads(self.buf.import_path().read_text())
			self.indent = 0
			self.on_open()

	def write(self, f, line: str = ""):
		f.write(f"{self.indent * '\t'}{line}\n")

	def write_enum(self, f, data: dict, real_key: EnumUtils.TRealKey, virtual_key: EnumUtils.TVirtualKey,
				   real_enum: Type[EnumUtils.TRealEnum], virtual_enum: Type[EnumUtils.TVirtualEnum]):
		value = EnumUtils.get_enum(data, real_key, real_enum, virtual_enum)
		self.write(f, f"{virtual_key.value} = <{value.value}> ( {', '.join(e.value for e in virtual_enum)} )")  # TODO v7.1 highlight default value from enum

	def write_int(self, f, data: dict, real_key: EnumUtils.TRealKey, virtual_key: EnumUtils.TVirtualKey, default: int, value_range: str):
		value = data.get(real_key.value, default)
		self.write(f, f"{virtual_key.value} = <{value}> ( {value_range} )")

	def write_float(self, f, data: dict, real_key: EnumUtils.TRealKey, virtual_key: EnumUtils.TVirtualKey, default: float, value_range: str):
		value = data.get(real_key.value, default)
		self.write(f, f"{virtual_key.value} = <{value}> ( {value_range} )")

	def write_bool(self, f, data: dict, real_key: EnumUtils.TRealKey, virtual_key: EnumUtils.TVirtualKey, default: bool):
		value = data.get(real_key.value, default)
		self.write(f, f"{virtual_key.value} = <{"true" if value else "false"}> ( true, false )")  # TODO v7.1 highlight default bool

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

	def log(self, *values, err=False) -> None:
		# TODO v7.2 use log file or new terminal instance for output
		if err:
			eprint(*values)
		else:
			print(*values)



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
