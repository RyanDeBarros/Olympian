import json
import sys
from pathlib import Path
from typing import Protocol, Any

DEFINITIONS_DIR = Path("definitions")
GEN_ROOT_DIR = Path("engine") / ".gen"


class GenFunc(Protocol):
	def __call__(self, file: Path, *args: Any, **kwargs: Any) -> list[str]:
		...


class CodeGen:
	def __init__(self, label: str):
		self.gen_dir = GEN_ROOT_DIR / label
		self.cache_file = GEN_ROOT_DIR / f".cache/{label}.json"
		self.def_dir = DEFINITIONS_DIR / label

		cache = self.cache_file.read_text() if self.cache_file.exists() else ""
		self.cache = json.loads(cache) if len(cache) > 0 else {}
		self.new_cache = {}
		self.files_seen: set[str] = set()

	def process(self, gen: GenFunc, rglob: str | None, /, *args, **kwargs):
		fail = False
		path_generator = self.def_dir.iterdir() if rglob is None else self.def_dir.rglob(rglob)
		for file in path_generator:
			if not self._process(file, gen, *args, **kwargs):
				fail = True

		self.finalize()

		if fail:
			sys.exit(1)

	def _process(self, file: Path, gen: GenFunc, /, *args, **kwargs) -> bool:
		cache_entry = file.relative_to(self.def_dir).with_suffix("").as_posix()
		self.files_seen.add(cache_entry)

		mtime = file.stat().st_mtime

		if cache_entry in self.cache and self.cache[cache_entry] == mtime:
			self.new_cache[cache_entry] = mtime
			return True

		errors = gen(file, *args, **kwargs)
		if errors:
			for error in errors:
				print(f"Code generation failed for {file.relative_to(self.def_dir).as_posix()}: {error}")
			return False
		else:
			self.new_cache[cache_entry] = mtime
			return True

	def finalize(self):
		self._prune()
		self._save()

	def _save(self):
		self.cache_file.parent.mkdir(parents=True, exist_ok=True)
		self.cache_file.touch(exist_ok=True)
		self.cache_file.write_text(json.dumps(self.cache))

	def _prune(self):
		for inl_file in self.gen_dir.rglob("*.inl"):
			cache_entry = inl_file.relative_to(self.gen_dir).with_suffix("").as_posix()
			if cache_entry not in self.files_seen:
				inl_file.unlink()

		for path in sorted(self.gen_dir.rglob("*"), reverse=True):
			if path.is_dir():
				try:
					path.rmdir()  # only works if empty
				except OSError:
					pass
