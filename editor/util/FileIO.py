import os
from pathlib import Path

import send2trash
import toml


def move_to_trash(path):
	send2trash.send2trash(Path(path).resolve())


def file_extension_filter(display, extensions):
	return f"{display} ({" ".join([f"*{ext}" for ext in extensions])})"


def get_files_of_certain_extensions(folder, extensions):
	files = set()
	for entry in os.listdir(folder):
		path = os.path.join(folder, entry)
		if os.path.isfile(path):
			_, ext = os.path.splitext(path)
			if ext in extensions:
				files.add(path)
	return files


def get_import_files_of_certain_extensions(folder, extensions, content_condition):
	imports = set()
	for entry in os.listdir(folder):
		path = os.path.join(folder, entry)
		if os.path.isfile(path) and path.endswith(".oly"):
			_, ext = os.path.splitext(path[:-len(".oly")])
			if ext in extensions:
				try:
					with open(path, 'r') as f:
						content = toml.load(f)
					if content_condition(content):
						imports.add(path)
				except toml.TomlDecodeError:
					pass
	return imports


def has_import_file(filepath, content_condition):
	if not os.path.exists(f"{filepath}.oly"):
		return False
	try:
		with open(f"{filepath}.oly", 'r') as f:
			content = toml.load(f)
		return content_condition(content)
	except toml.TomlDecodeError:
		return False


def create_default_import_file(filepath, d):
	with open(f"{filepath}.oly", 'w') as f:
		toml.dump(d, f)


# TODO v3 allow selection of specific (even multiple) files, rather than folder
def execute_standard_import_on_folder(root_folder, recursive, clear, clean_unused, import_unimported, extensions,
									  content_condition, create_default_import):
	def execute_on_folder(folder):
		if clear:
			import_files = get_import_files_of_certain_extensions(folder, extensions, content_condition)
			for import_file in import_files:
				move_to_trash(import_file)
		elif clean_unused:
			import_files = get_import_files_of_certain_extensions(folder, extensions, content_condition)
			for import_file in import_files:
				if not os.path.exists(import_file[:-len(".oly")]):
					move_to_trash(import_file)
		if import_unimported:
			asset_files = get_files_of_certain_extensions(folder, extensions)
			for asset_file in asset_files:
				if not has_import_file(asset_file, content_condition):
					create_default_import(asset_file)

	if recursive:
		for root, dirs, files in os.walk(root_folder):
			execute_on_folder(root)
	else:
		execute_on_folder(root_folder)
