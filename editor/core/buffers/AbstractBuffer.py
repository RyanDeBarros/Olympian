from abc import ABC, abstractmethod
from contextlib import contextmanager
from pathlib import Path
from typing import Optional

import toml

from editor.core import FileSystemWatcher
from editor.tools import eprint, TOMLAdapter
from . import BufferPath
from .processing import Metadata, AssetType, EnumField, RangedNumberField, DiscreteNumberField, BoolField
from .processing.BufferSection import BufferSection


class AbstractBuffer(FileSystemWatcher, ABC):
	def __init__(self, buf: BufferPath):
		super().__init__()
		self.buf = buf
		self.d = {}
		self.indent = 0

		self.root_section = BufferSection("", None)
		self.subsections: list[BufferSection] = []
		self.current_section = None

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

	def format_version(self) -> float:
		return Metadata.version(self.buf.metadata())

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
		if len(line) > 0:
			f.write(f"{self.indent * '\t'}{line}\n")
		else:
			f.write('\n')

	def write_enum(self, f, data: dict, field: EnumField):
		self.current_section.fields.append(field.virtual_key.value)
		value = field.get_value(data)
		self.write(f, f"{field.virtual_key.value}: {value}")
		self.indent += 1
		self.write(f, f"options: {field.options}\n")
		if len(field.description) > 0:
			self.write(f, f"description: {field.description}")  # TODO v7.1 make description hidden by default until !info?
		self.indent -= 1

	def write_ranged_number(self, f, data: dict, field: RangedNumberField):
		self.current_section.fields.append(field.virtual_key.value)
		value = field.get_value(data)
		self.write(f, f"{field.virtual_key.value}: {value}")
		self.indent += 1
		self.write(f, f"range: {field.options}")
		if field.default is not None:
			self.write(f, f"default: {field.default}")
		if len(field.description) > 0:
			self.write(f, f"description: {field.description}")
		self.write(f)
		self.indent -= 1

	def write_discrete_number(self, f, data: dict, field: DiscreteNumberField):
		self.current_section.fields.append(field.virtual_key.value)
		value = field.get_value(data)
		self.write(f, f"{field.virtual_key.value}: {value}")
		self.indent += 1
		self.write(f, f"options: {field.options}")
		if len(field.description) > 0:
			self.write(f, f"description: {field.description}")
		self.write(f)
		self.indent -= 1

	def write_bool(self, f, data: dict, field: BoolField):
		self.current_section.fields.append(field.virtual_key.value)
		value = field.get_value(data)
		self.write(f, f"{field.virtual_key.value}: {'true' if value else 'false'}")
		self.indent += 1
		self.write(f, f"options: {field.options}")
		if len(field.description) > 0:
			self.write(f, f"description: {field.description}")
		self.write(f)
		self.indent -= 1

	@contextmanager
	def write_subsection(self, f, name: str):
		parent = self.current_section
		section = BufferSection(name, parent)
		self.current_section = section
		self.write(f, self.current_section.title())
		yield section
		self.current_section = parent

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
