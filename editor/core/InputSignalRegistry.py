from pathlib import Path

import toml


class InputSignalRegistry:
	from editor.core.ProjectContext import ProjectContext
	def __init__(self, project_context: ProjectContext):
		self.project_context = project_context
		self._signal_items: list[Path] = []
		self.load()

	# TODO v3 with custom formats, loading/dumping can be more efficient

	def load(self):
		with open(self.project_context.project_file, 'r') as f:
			content = toml.load(f)
		if 'context' in content and 'signals' in content['context']:
			self._signal_items = [Path(path) for path in content['context']['signals']]
		else:
			self._signal_items = []

	def dump(self):
		with open(self.project_context.project_file, 'r') as f:
			content = toml.load(f)
		if 'context' not in content:
			content['context'] = {}
		content['context']['signals'] = [path.as_posix() for path in self._signal_items]
		with open(self.project_context.project_file, 'w') as f:
			toml.dump(content, f)

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
