from abc import ABC, abstractmethod

import toml

from ToolNode import *


class Importer(ABC):
    @abstractmethod
    def __init__(self, folder: str, recur: bool, prune: bool, default: bool, clear: bool):
        self.folder = res_path(folder)
        self.recur = recur
        self.prune = prune
        self.default = default
        self.clear = clear
        self.asset_file_extensions = None
        self.default_toml = {}

    def setup(self, package: str, asset_file_extensions: tuple[str, ...]):
        self.asset_file_extensions = asset_file_extensions
        default_import_file = os.path.join(package, 'default.toml')
        if os.path.exists(default_import_file):
            with open(default_import_file, 'r') as f:
                self.default_toml = toml.load(f)

    def run(self):
        if self.clear:
            self.run_clear(self.folder)
        else:
            self.run_search(self.folder)

    def prune_import(self, file: str):
        if self.nonoly_filepath(file).endswith(self.asset_file_extensions) and not os.path.exists(self.nonoly_filepath(file)):
            os.remove(file)

    @staticmethod
    def nonoly_filepath(file: str) -> str:
        return file[:-len('.oly')]

    def run_clear(self, folder: str):
        with os.scandir(folder) as entries:
            for entry in entries:
                if entry.is_file():
                    if entry.path.endswith('.oly') and self.nonoly_filepath(entry.path).endswith(self.asset_file_extensions):
                        os.remove(entry.path)
                elif entry.is_dir() and self.recur:
                    self.run_clear(entry.path)

    def run_search(self, folder: str):
        with os.scandir(folder) as entries:
            for entry in entries:
                if entry.is_file():
                    if entry.path.endswith('.oly'):
                        if self.prune:
                            self.prune_import(entry.path)
                    else:
                        self.import_file(entry.path)
                elif entry.is_dir() and self.recur:
                    self.run_search(entry.path)

    def import_file(self, asset_file: str):
        if not asset_file.endswith(self.asset_file_extensions):
            return

        if os.path.exists(asset_file + '.oly'):
            with open(asset_file + '.oly', 'r') as f:
                try:
                    tml = toml.load(f)
                except toml.TomlDecodeError:
                    tml = {}
        else:
            tml = {}
        if self.write_default_toml(asset_file, tml):
            with open(asset_file + '.oly', 'w') as f:
                toml.dump(tml, f)

    @abstractmethod
    def write_default_toml(self, file: str, tml: dict) -> bool:
        pass


def import_folder(prompt: str, importer: type[Importer]):
    folder = ""
    while len(folder) == 0:
        folder = varinput(prompt)

    recur = varinput("Recursive search (y/n): ") == "y"
    prune = varinput("Prune isolated imports (y/n): ") != "n"
    default = varinput("Reset existing imports to default (y/n): ") == "y"
    clear = varinput("Clear imports (y/n): ") == "y"

    importer(folder=folder, recur=recur, prune=prune, default=default, clear=clear).run()


def import_manifest(package: str, importer: type[Importer]):
    with open(os.path.join(package, 'manifest.toml'), 'r') as f:
        folders = toml.load(f)["folder"]

    for folder in folders:
        if 'path' not in folder:
            continue
        recur = folder['recur'] if 'recur' in folder else False
        prune = folder['prune'] if 'prune' in folder else True
        default = folder['default'] if 'default' in folder else False
        clear = folder['clear'] if 'clear' in folder else False
        # TODO these parameters should only use deepest manifest settings for the folder, i.e., if a defines default=true, and a/b defines default=false, then use default=false within a/b and default=true elsewhere in a.
        importer(folder=folder['path'], recur=recur, prune=prune, default=default, clear=clear).run()
