from pathlib import Path

from editor.core.context import EditorContext


class EditorSettings:
	def __init__(self, project_dir: Path):
		self.project_dir = project_dir

	def persistent_path(self) -> Path:
		return EditorContext.context_root(self.project_dir) / f'settings.{EditorContext.BUFFER_FILE_EXTENSION}'
