import argparse
import os
import toml

from Tool import ToolNode, print_info, varinput

IMPORT_FILE_EXTENSIONS = (".png", ".jpg", ".jpeg", ".bmp", ".svg", ".gif")
IMPORT_FILE_EXTENSIONS_IMAGES = (".png", ".jpg", ".jpeg", ".bmp")


class TextureImporter:
    def __init__(self, folder: str, recur: bool, prune: bool, default: bool, clear: bool):
        self.folder = os.path.join("../res/", folder)
        self.recur = recur
        self.prune = prune
        self.default = default
        self.clear = clear
        with open('DefaultTextureImport.toml', 'r') as f:
            self.default_toml = toml.load(f)

    def run(self):
        if self.clear:
            self.run_clear(self.folder)
        else:
            self.run_search(self.folder)

    def run_clear(self, folder: str):
        with os.scandir(folder) as entries:
            for entry in entries:
                if entry.is_file():
                    if entry.path.endswith('.oly') and self.nonoly_filepath(entry.path).endswith(
                            IMPORT_FILE_EXTENSIONS):
                        with open(entry.path, 'r') as f:
                            tml = toml.load(f)
                        if 'texture' in tml:
                            del tml['texture']
                            with open(entry.path, 'w') as f:
                                toml.dump(tml, f)
                        if len(tml) == 0:
                            os.remove(entry.path)
                elif entry.is_dir() and self.recur:
                    self.run_clear(entry.path)

    def run_search(self, folder: str):
        with os.scandir(folder) as entries:
            for entry in entries:
                if entry.is_file():
                    if self.prune and entry.path.endswith('.oly'):
                        self.prune_imports(entry.path)
                    elif entry.path.endswith(IMPORT_FILE_EXTENSIONS):
                        self.import_texture(entry.path)
                elif entry.is_dir() and self.recur:
                    self.run_search(entry.path)

    def prune_imports(self, file: str):
        if self.nonoly_filepath(file).endswith(IMPORT_FILE_EXTENSIONS) and not os.path.exists(
                self.nonoly_filepath(file)):
            os.remove(file)

    def nonoly_filepath(self, file: str):
        return file[:-len('.oly')]

    def import_texture(self, file: str):
        if file.endswith(IMPORT_FILE_EXTENSIONS_IMAGES):
            self.write_default(file, 'image')
        elif file.endswith(".svg"):
            self.write_default(file, 'svg')
        elif file.endswith(".gif"):
            self.write_default(file, 'gif')

    def write_default(self, file: str, texture_type: str):
        if os.path.exists(file + '.oly'):
            with open(file + '.oly', 'r') as f:
                tml = toml.load(f)
        else:
            tml = {}

        if 'texture' in tml:
            if self.default:
                tml['texture'] = [{}]
            else:
                return
        else:
            tml['texture'] = [{}]

        for k, v in self.default_toml[texture_type].items():
            tml['texture'][0][k] = v
        with open(file + '.oly', 'w') as f:
            toml.dump(tml, f)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Auto-generates .oly texture import files with default parameters "
                                                 "for isolated textures.")

    parser.add_argument('-f', '--folder', required=True, type=str, help="Folder to look in")
    parser.add_argument('-r', '--recur', action='store_true', help="Recursively search subfolders")
    parser.add_argument('-p', '--prune', action='store_true', help="Remove isolated import files")
    parser.add_argument('-d', '--default', action='store_true', help="Reset existing texture import files to default")
    parser.add_argument('-c', '--clear', action='store_true', help="Clear all texture import files")

    args = parser.parse_args()
    TextureImporter(args.folder, args.recur, args.prune, args.default, args.clear).run()


def import_textures():
    folder = ""
    while len(folder) == 0:
        folder = varinput("Textures folder: ")

    recur = varinput("Recursive search (y/n): ") == "y"
    prune = varinput("Prune isolated imports (y/n): ") != "n"
    default = varinput("Reset existing imports to default (y/n): ") == "y"
    clear = varinput("Clear imports (y/n): ") == "y"

    TextureImporter(folder=folder, recur=recur, prune=prune, default=default, clear=clear).run()
    print_info("success!")


def edit_defaults():
    print_info("TODO editing defaults...")  # TODO


TOOL = ToolNode("textures", "Manipulate texture import (.oly) files.")

IMPORT_TEXTURES = ToolNode("import", "Generate import files.", import_textures)
TOOL.add_child(IMPORT_TEXTURES)

EDIT_DEFAULTS = ToolNode("edit defaults", "Edit the default texture import structure.", edit_defaults)
TOOL.add_child(EDIT_DEFAULTS)
