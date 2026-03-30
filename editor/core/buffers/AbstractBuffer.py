from abc import ABC, abstractmethod
from contextlib import contextmanager
from pathlib import Path
from typing import Optional, Iterator

import toml

from editor.core import FileSystemWatcher
from editor.tools import eprint, TOMLAdapter
from . import BufferPath
from .processing import *
from ..context import PathUtils


class AbstractBuffer(FileSystemWatcher, ABC):
	def __init__(self, buf: BufferPath):
		super().__init__()
		self.buf = buf
		self.internally_modified = False
		self.last_asset_hash = None
		self.last_buffer_hash = None

		self.d = {}
		self.stream = BufferStream()
		self.commands: list[ExclamCommand] = []

		self.tree = BufferTree()

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
			asset_hash = PathUtils.file_hash(self.buf.asset_path)
			if asset_hash != self.last_asset_hash:
				self.last_asset_hash = asset_hash
				self.do_open()
		elif path == self.buf.buffer_path:
			if self.internally_modified:
				self.internally_modified = False
				self.last_buffer_hash = PathUtils.file_hash(self.buf.buffer_path)
			else:
				buffer_hash = PathUtils.file_hash(self.buf.buffer_path)
				if buffer_hash != self.last_buffer_hash:
					self.last_buffer_hash = buffer_hash
					self.load_root_section()
					self.on_buffer_modified()

	def on_buffer_modified(self) -> None:
		pass

	def on_deleted(self, path: Path, was_dir: bool) -> None:
		if path == self.buf.asset_path:
			self.on_asset_deleted()

	def on_asset_deleted(self) -> None:
		pass

	def on_moved(self, src: Path, dest: Path, was_dir: bool) -> None:
		if src == self.buf.asset_path:
			if self.__class__.matches_asset(BufferPath.from_asset(dest)):
				pass  # TODO v7
			# self.buf.on_asset_moved(dest)
			else:
				eprint(f"{self.__class__.__name__}: Not Implemented")  # TODO v7.2 handle different asset type - also, self.close() here might not work with the asset file moved

	def open(self) -> None:
		if not self.buf.buffer_path.exists():
			self.buf.buffer_path.parent.mkdir(parents=True, exist_ok=True)
			self.buf.buffer_path.touch()
			self.do_open()

	def do_open(self) -> None:
		self.d = toml.loads(self.buf.import_path().read_text())
		self.stream.reset()
		self.tree.reset()
		self.write_buffer()
		self.flush_write()

	def flush_write(self):
		with self.buf.buffer_path.open('w') as w:
			self.internally_modified = True
			w.write(self.stream.string())

	@contextmanager
	def write_subsection(self, name: str) -> Iterator[BufferSection]:
		parent = self.tree.current_section
		section = BufferSection(name, parent)
		self.tree.current_section = section
		with self.stream.subindent():
			self.stream.write_subsection(self.tree.current_section.title())
			yield section
		self.tree.current_section = parent

	def write_meta_block(self, header: str) -> None:
		with self.stream.subindent():
			self.stream.write(BufferParseStructure.META_BLOCK_DELIMITER)
			self.stream.write(header)
			self.stream.write(f"Format version: {self.format_version()}")
			self.stream.write(f"\n!commands:")
			with self.stream.subindent():
				for command in self.commands:
					self.stream.write(f"{command.exclam}: {command.info}")
			self.stream.write(BufferParseStructure.META_BLOCK_DELIMITER)

	def write_field(self, key):
		self.stream.write()
		self.stream.write(self.tree.current_section.repr_field(key))

	def write_field_metadata(self, metadata: dict[str, str]):
		with self.stream.subindent():
			for name, data in metadata.items():
				if data is not None and (not isinstance(data, str) or len(data) > 0):
					self.stream.write(self.tree.current_section.repr_metadata(f"{name}: {data}"))

	def write_enum(self, data: dict, field: EnumField):
		key = field.virtual_key.value
		self.tree.current_section.fields[key] = field.get_value(data).value
		self.write_field(key)
		self.write_field_metadata({"options": field.options, "description": field.description})  # TODO v7.1 make description hidden by default until !info?

	def write_ranged_number(self, data: dict, field: RangedNumberField):
		key = field.virtual_key.value
		self.tree.current_section.fields[key] = field.get_value(data)
		self.write_field(key)
		self.write_field_metadata({"range": field.range, "default": field.default, "description": field.description})

	def write_discrete_number(self, data: dict, field: DiscreteNumberField):
		key = field.virtual_key.value
		self.tree.current_section.fields[key] = field.get_value(data)
		self.write_field(key)
		self.write_field_metadata({"options": field.options, "description": field.description})

	def write_bool(self, data: dict, field: BoolField):
		key = field.virtual_key.value
		self.tree.current_section.fields[key] = 'true' if field.get_value(data) else 'false'
		self.write_field(key)
		self.write_field_metadata({"options": field.options, "description": field.description})

	def rebuild_root_section(self) -> list[DeferredExclam]:
		return self.tree.rebuild(self.commands, self.buf.buffer_path.read_text().split('\n'))

	def load_root_section(self):
		deferred_exclams = self.rebuild_root_section()

		if len(deferred_exclams) > 0:
			self.stream.reset()
			self.stream.raw_write(self.buf.buffer_path.read_text())

			exclam_edits: list[ExclamEdit] = []
			for deferred_exclam in deferred_exclams:
				exclam_edits.append(deferred_exclam.removal())

			for deferred_exclam in deferred_exclams:
				exclam_edits += deferred_exclam.invoke(self.commands)

			while len(exclam_edits) > 0:
				current_edit = exclam_edits.pop(0)
				current_edit.invoke(self.stream.fio, exclam_edits)
			self.flush_write()

			deferred_exclams = self.rebuild_root_section()
			if len(deferred_exclams) > 0:
				pass  # TODO v7.1 warn/error new commands appeared somehow

	def write_buffer(self) -> None:
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
