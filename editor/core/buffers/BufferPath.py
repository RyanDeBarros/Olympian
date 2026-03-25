from pathlib import Path

from editor.core import ProgramState


class BufferPath:
	def __init__(self, *, asset_path: Path, buffer_path: Path):
		self.asset_path = asset_path
		self.buffer_path = buffer_path

	@classmethod
	def from_asset(cls, asset_path: Path) -> "BufferPath":
		from editor.core.context import EditorContext
		rel_path = asset_path if not asset_path.is_absolute() else asset_path.relative_to(ProgramState.instance().project_dir)
		buffer_path = EditorContext.data_root() / f"{rel_path}.{EditorContext.BUFFER_FILE_EXTENSION}"
		return cls(asset_path=asset_path, buffer_path=buffer_path)

	@classmethod
	def from_buffer(cls, buffer_path: Path) -> "BufferPath":
		from editor.core.context import EditorContext
		rel_path = buffer_path.relative_to(EditorContext.data_root()).with_suffix("")
		asset_path = ProgramState.instance().project_dir / rel_path
		return cls(asset_path=asset_path, buffer_path=buffer_path)
