import subprocess
from asyncio import Future
from concurrent.futures.thread import ThreadPoolExecutor
from contextlib import contextmanager
from pathlib import Path
from typing import Generator

import toml

CACHE_FILE = Path('.dev/qtconv.toml')
SEARCH_FOLDER = Path('ui')


@contextmanager
def caching() -> Generator[dict, None, None]:
	if CACHE_FILE.exists():
		with open(CACHE_FILE, 'r') as f:
			cache = toml.load(f).get("files", {})
	else:
		cache = {}
	try:
		yield cache
	finally:
		cache = {k: v for k, v in cache.items() if Path(k).exists()}
		CACHE_FILE.parent.mkdir(parents=True, exist_ok=True)
		with open(CACHE_FILE, 'w') as f:
			toml.dump({'files': cache}, f)


def convert(ui_path: Path, py_path: Path):
	print(f"Converting {ui_path.as_posix()} -> {py_path.as_posix()}")
	subprocess.run(['pyside6-uic', ui_path, '-o', py_path], check=True)


def run():
	with caching() as cache:
		tasks: list[tuple[str, int, Future]] = []
		with ThreadPoolExecutor() as executor:
			for ui_path in SEARCH_FOLDER.rglob('*.ui'):
				py_path = ui_path.with_suffix('.py')
				key = ui_path.as_posix()
				mtime = int(ui_path.stat().st_mtime)
				if cache.get(key) != mtime:
					tasks.append((key, mtime, executor.submit(convert, ui_path, py_path)))

			for key, mtime, future in tasks:
				future.result()
				cache[key] = mtime


if __name__ == "__main__":
	run()
