from pathlib import Path

from editor import TOMLAdapter


def get_path_item(path: Path):
	from editor.core import FolderPathItem, StandardFilePathItem, InputSignalPathItem, ImportedTexturePathItem, ImportedFontPathItem
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
			import_path = Path(path.as_posix() + '.oly')
			if import_path.exists():
				meta = TOMLAdapter.meta(import_path)
				if 'type' in meta:
					match meta['type']:
						case 'texture':
							return ImportedTexturePathItem(path)
						case 'font':
							return ImportedFontPathItem(path)
				return None
			return StandardFilePathItem(path)
	else:
		return None
