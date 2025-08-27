from pathlib import Path

import toml


def get_path_item(path: Path):
	from editor.core import FolderPathItem, StandardFilePathItem, InputSignalPathItem
	if path.is_dir():
		return FolderPathItem(path)
	elif path.is_file():
		# TODO v3 peek file to get type. With TOML, must load entire file, but with custom format, can peek to N characters.
		if path.suffix == ".oly":
			with open(path, 'r') as f:
				d = toml.load(f)
			if 'header' in d and d['header'] == 'signal':
				return InputSignalPathItem(path)
			return None  # TODO v3 add if asset and not import
		else:
			# TODO v3 check for existing import file - use ImportedFilePathItem, which still refers to file, not import file, but recognizes that there exists an import.
			return StandardFilePathItem(path)
	else:
		return None
