from pathlib import Path

from editor.tools import TOMLAdapter


class InputSignalRegistry:
	from editor.core.ProjectContext import ProjectContext
	def __init__(self, project_context: ProjectContext):
		self.project_context = project_context
		self._signal_paths: list[Path] = []
		self.load()

	def load(self):
		content = TOMLAdapter.load(self.project_context.project_file)
		if 'context' in content and 'signals' in content['context']:
			self._signal_paths = [Path(path) for path in content['context']['signals'] if Path(path).exists()]
		else:
			self._signal_paths = []

	def dump(self):
		content = TOMLAdapter.load(self.project_context.project_file)
		if 'context' not in content:
			content['context'] = {}
		content['context']['signals'] = [path.as_posix() for path in self._signal_paths if path.exists()]
		TOMLAdapter.dump(self.project_context.project_file, content)

	def has_signal_asset(self, asset: Path):
		return asset in self._signal_paths

	def add_signal_asset(self, asset: Path):
		assert asset not in self._signal_paths
		self._signal_paths.append(asset)
		self.dump()

	def remove_signal_asset(self, asset: Path):
		assert asset in self._signal_paths
		self._signal_paths.remove(asset)
		self.dump()

	def rename_signal_asset(self, from_asset: Path, to_asset: Path):
		assert from_asset in self._signal_paths and to_asset not in self._signal_paths
		index = self._signal_paths.index(from_asset)
		self._signal_paths[index] = to_asset
		self.dump()
