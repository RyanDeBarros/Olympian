from pathlib import Path

from editor import TOMLAdapter


class InputSignalRegistry:
	from editor.core.ProjectContext import ProjectContext
	def __init__(self, project_context: ProjectContext):
		self.project_context = project_context
		self._signal_items: list[Path] = []
		self.load()

	def load(self):
		content = TOMLAdapter.load(self.project_context.project_file)
		if 'context' in content and 'signals' in content['context']:
			self._signal_items = [Path(path) for path in content['context']['signals']]
		else:
			self._signal_items = []

	def dump(self):
		content = TOMLAdapter.load(self.project_context.project_file)
		if 'context' not in content:
			content['context'] = {}
		content['context']['signals'] = [path.as_posix() for path in self._signal_items]
		TOMLAdapter.dump(self.project_context.project_file, content)

	def has_signal_asset(self, asset: Path):
		return asset in self._signal_items

	def add_signal_asset(self, asset: Path):
		assert asset not in self._signal_items
		self._signal_items.append(asset)
		self.dump()

	def remove_signal_asset(self, asset: Path):
		assert asset in self._signal_items
		self._signal_items.remove(asset)
		self.dump()

	def rename_signal_asset(self, from_asset: Path, to_asset: Path):
		assert from_asset in self._signal_items and to_asset not in self._signal_items
		index = self._signal_items.index(from_asset)
		self._signal_items[index] = to_asset
		self.dump()
