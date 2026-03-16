import json
from pathlib import Path

from editor.filesystem import AppData

TEMP: dict[str, str] = {}
PERSISTENT_PATH: Path = AppData.PERSISTENT_PATH / 'vars/persistent.json'

PERSISTENT: dict[str, str] = json.loads(PERSISTENT_PATH.read_text()) if PERSISTENT_PATH.exists() else {}


def get_temp(key: str) -> str:
	global TEMP
	return TEMP[key]


def set_temp(key: str, value: str):
	global TEMP
	TEMP[key] = value


def del_temp(key: str):
	global TEMP
	if key in TEMP:
		del TEMP[key]


def clear_temp():
	global TEMP
	TEMP.clear()


def temp_keys() -> list[str]:
	global TEMP
	return list(TEMP.keys())


def _dump_persistent():
	if not PERSISTENT_PATH.exists():
		PERSISTENT_PATH.parent.mkdir(parents=True, exist_ok=True)
		PERSISTENT_PATH.touch()
	PERSISTENT_PATH.write_text(json.dumps(PERSISTENT))


def get_persistent(key: str) -> str:
	global PERSISTENT
	return PERSISTENT[key]


def set_persistent(key: str, value: str):
	global PERSISTENT
	PERSISTENT[key] = value
	_dump_persistent()


def del_persistent(key: str):
	global PERSISTENT
	if key in PERSISTENT:
		del PERSISTENT[key]
		_dump_persistent()


def clear_persistent():
	global PERSISTENT
	PERSISTENT.clear()
	_dump_persistent()


def persistent_keys() -> list[str]:
	global PERSISTENT
	return list(PERSISTENT.keys())
