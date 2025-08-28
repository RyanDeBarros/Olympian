from pathlib import Path

from editor import TOMLAdapter


def get_path_item(path: Path):
	from editor.core import FolderPathItem, StandardFilePathItem, InputSignalPathItem
	if path.is_dir():
		return FolderPathItem(path)
	elif path.is_file():
		if path.suffix == ".oly":
			meta = TOMLAdapter.meta(path)
			if 'type' in meta:
				match meta['type']:
					case 'signal':
						return InputSignalPathItem(path)
			return None
		else:
			import_path = Path(str(path) + '.oly')
			if import_path.exists():
				meta = TOMLAdapter.meta(import_path)
				# TODO v3 Use ImportedFilePathItem, which still refers to file, not import file, but recognizes that there exists an import.
				if 'type' in meta:
					match meta['type']:
						case 'texture':
							pass  # TODO v3
						case 'font':
							pass  # TODO v3
				return None
			return StandardFilePathItem(path)
	else:
		return None
