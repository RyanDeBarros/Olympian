from pathlib import Path

from .processing import Metadata, AssetType
from editor.tools import TOMLAdapter


class BufferPath:
	def __init__(self, *, asset_path: Path, buffer_path: Path):
		self.asset_path = asset_path
		self.buffer_path = buffer_path

	def is_imported(self) -> bool:
		return self.asset_path.suffix != Metadata.IMPORT_EXTENSION

	def asset_type(self) -> AssetType | None:
		p = self.import_path() if self.is_imported() else self.asset_path
		return Metadata.asset_type(TOMLAdapter.meta(p)) if p.exists() else None

	def import_path(self) -> Path | None:
		if self.is_imported():
			return self.asset_path.with_name(self.asset_path.name + Metadata.IMPORT_EXTENSION)
		else:
			return None

	@staticmethod
	def convert_to_buffer_path(asset_path: Path) -> Path:
		from editor.core.context import EditorContext
		from editor.core import ProgramState
		if asset_path.suffix == Metadata.IMPORT_EXTENSION and Metadata.is_import(TOMLAdapter.meta(asset_path)):  # redirect import file to asset file
			asset_path = asset_path.with_suffix("")
		rel_path = asset_path if not asset_path.is_absolute() else asset_path.relative_to(ProgramState.instance().project_dir)
		return EditorContext.data_root() / f"{rel_path}.{EditorContext.BUFFER_FILE_EXTENSION}"

	@staticmethod
	def convert_to_asset_path(buffer_path: Path) -> Path:
		from editor.core.context import EditorContext
		from editor.core import ProgramState
		rel_path = buffer_path.relative_to(EditorContext.data_root()).with_suffix("")
		return ProgramState.instance().project_dir / rel_path

	@classmethod
	def from_asset(cls, asset_path: Path) -> "BufferPath":
		return cls(asset_path=asset_path, buffer_path=BufferPath.convert_to_buffer_path(asset_path))

	@classmethod
	def from_buffer(cls, buffer_path: Path) -> "BufferPath":
		return cls(asset_path=BufferPath.convert_to_asset_path(buffer_path), buffer_path=buffer_path)

	def on_asset_moved(self, asset_path: Path):
		self.asset_path = asset_path
		self.buffer_path.rename(BufferPath.convert_to_buffer_path(self.asset_path))
