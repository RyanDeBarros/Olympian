import hashlib
from pathlib import Path


def printed_path(path: Path):
	return str(path)  # TODO v7.2 editor setting to print as str(path) or path.as_posix()


def file_hash(path: Path) -> str:
	return hashlib.md5(path.read_bytes()).hexdigest()
