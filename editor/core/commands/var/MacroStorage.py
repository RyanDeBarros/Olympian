import re
from pathlib import Path

from editor.core import Resolver
from editor.core.context import EditorContext


class MacroDict:
	def __init__(self, dump):
		self._d = {}
		self._dump = dump

	def dump(self) -> None:
		if self._dump is not None:
			self._dump(self._d)

	def get(self, key: str) -> str:
		return self._d[key]

	def set(self, key: str, val: str) -> None:
		self._d[key] = val
		self.dump()

	def remove(self, key: str) -> None:
		if key in self._d:
			del self._d[key]
			self.dump()

	def clear(self) -> None:
		self._d.clear()
		self.dump()

	def keys(self) -> list[str]:
		return list(self._d.keys())


class MacroStorage:
	def __init__(self, project_dir: Path):
		self.project_dir = project_dir
		self.temporary = MacroDict(None)
		self.persistent = MacroDict(self.dump_persistent)
		if self.persistent_path().is_file():
			self.load_persistent()

	def persistent_path(self) -> Path:
		return EditorContext.context_root(self.project_dir) / f'macros.{EditorContext.BUFFER_FILE_EXTENSION}'

	def dump_persistent(self, d: dict[str, str]) -> None:
		self.persistent_path().write_text('\n'.join(f"{k} = {Resolver.GROUP_OPEN}{v}{Resolver.GROUP_CLOSE}" for k, v in d.items()))

	def load_persistent(self) -> None:
		pattern = re.compile(rf"^(?P<key>.*?) = {re.escape(Resolver.GROUP_OPEN)}(?P<val>.*){re.escape(Resolver.GROUP_CLOSE)}$")
		with self.persistent_path().open('r') as f:
			for line in f:
				match = pattern.match(line)
				if match:
					self.persistent.set(match.group("key"), match.group("val"))
