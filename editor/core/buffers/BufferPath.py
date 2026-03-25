from pathlib import Path


class BufferPath:
	def __init__(self, *, asset_path: Path, buffer_path: Path):
		self.asset_path = asset_path
		self.buffer_path = buffer_path

	@staticmethod
	def convert_to_buffer_path(asset_path: Path) -> Path:
		from editor.core.context import EditorContext
		from editor.core import ProgramState
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
