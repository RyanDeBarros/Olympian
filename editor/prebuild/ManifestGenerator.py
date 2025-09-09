import json
import os
import shutil
from pathlib import Path

from editor.prebuild import Archetype
from editor.tools import TOMLAdapter

ARCHETYPE_GEN_PATH = Path('.gen/archetypes').resolve()


# TODO v4 move Cache and manifest generator into different file from source code generator above
class Cache:
	def __init__(self):
		self.cache_path = Path('.gen/cache.json').resolve()
		self.cache: dict[str, float] = {}
		if self.cache_path.exists():
			with open(self.cache_path, 'r') as f:
				try:
					self.cache: dict = json.load(f)
				except json.JSONDecodeError:
					pass
		self.marked: list[str] = []

	def dump(self):
		self.cache = {file: self.cache[file] for file in self.marked}
		self.marked.clear()
		with open(self.cache_path, 'w') as f:
			json.dump(self.cache, f)

	def is_dirty(self, file: Path) -> bool:
		return file.as_posix() not in self.cache or self.cache[file.as_posix()] != file.stat().st_mtime

	def update(self, file: Path):
		self.cache[file.as_posix()] = file.stat().st_mtime

	def mark(self, file: Path):
		self.marked.append(file.as_posix())

	def clear(self):
		self.cache.clear()
		self.marked.clear()
		self.dump()
		shutil.rmtree(ARCHETYPE_GEN_PATH)
		os.makedirs(ARCHETYPE_GEN_PATH)


def generate_manifest():
	def generate_file(file: Path):
		if TOMLAdapter.meta(file).get('type') == 'archetype':
			cache.mark(file)
			if cache.is_dirty(file):
				Archetype.generate_archetype(file, ARCHETYPE_GEN_PATH)
				cache.update(file)

	def generate_folder(folder):
		for file in Path(folder).rglob("*.toml"):
			generate_file(file)

	assets = Path('.gen/manifest.txt').read_text().splitlines()
	cache = Cache()
	for asset in assets:
		asset_path = Path(f"res/{asset}")
		if asset_path.is_file():
			generate_file(asset_path)
		elif asset_path.is_dir():
			generate_folder(asset_path)
	cache.dump()
