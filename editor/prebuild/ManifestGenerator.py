import json
import os
import shutil
from pathlib import Path

from editor.prebuild import Archetype
from editor.tools import TOMLAdapter

ARCHETYPE_RES_PATH = Path('res')
ARCHETYPE_GEN_PATH = Path('.gen/code')


class Cache:
	def __init__(self):
		self.cache_path = Path('.gen/cache.json').resolve()
		self.cache: dict[str, float] = {}
		if self.cache_path.exists():
			try:
				json.loads(self.cache_path.read_text())
			except json.JSONDecodeError:
				pass
		self.marked: list[str] = []

	def dump(self):
		self.prune()
		self.cache = {file: self.cache[file] for file in self.marked}
		self.marked.clear()
		self.cache_path.write_text(json.dumps(self.cache))

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

	def prune(self):
		for path in ARCHETYPE_GEN_PATH.rglob("*"):
			if path.is_file():
				if (ARCHETYPE_RES_PATH / path.relative_to(ARCHETYPE_GEN_PATH).with_suffix(".toml")).as_posix() not in self.marked:
					path.unlink()

		for path in sorted(ARCHETYPE_GEN_PATH.rglob("*"), reverse=True):
			if path.is_dir() and not any(path.iterdir()):
				path.rmdir()


def generate_manifest():
	def generate_file(file: Path):
		if TOMLAdapter.meta(file).get('type') == 'archetype':
			cache.mark(file)
			if cache.is_dirty(file):
				Archetype.generate_archetype(file.resolve(), ARCHETYPE_GEN_PATH, ARCHETYPE_RES_PATH)
				cache.update(file)

	def generate_folder(folder):
		for file in Path(folder).rglob("*.toml"):
			generate_file(file)

	assets = Path('.gen/manifest.txt').read_text().splitlines()
	cache = Cache()
	for asset in assets:
		asset_path = ARCHETYPE_RES_PATH / asset
		if asset_path.is_file():
			generate_file(asset_path)
		elif asset_path.is_dir():
			generate_folder(asset_path)
	cache.dump()
