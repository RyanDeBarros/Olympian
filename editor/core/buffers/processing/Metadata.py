from enum import Enum

IMPORT_EXTENSION = '.oly'


class AssetType(Enum):
	TEXTURE = 'texture'


def is_import(m: dict) -> bool:
	return m['import']


def asset_type(m: dict) -> AssetType:
	return AssetType(m['type'])


def version(m: dict) -> float:
	return m['version']
