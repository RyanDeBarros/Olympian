import argparse
import os

import toml

from Tool import *
from Parameters import *

IMPORT_FILE_EXTENSIONS = (".png", ".jpg", ".jpeg", ".bmp", ".svg", ".gif")
IMPORT_FILE_EXTENSIONS_IMAGES = (".png", ".jpg", ".jpeg", ".bmp")
IMPORT_FILE_EXTENSIONS_SPRITESHEETABLE = (".png", ".jpg", ".jpeg", ".bmp", ".svg")
DEFAULT_TEXTURE_IMPORT_FILE = "DefaultTextureImport.toml"


class TextureImporter:
    def __init__(self, folder: str, recur: bool, prune: bool, default: bool, clear: bool):
        self.folder = res_path(folder)
        self.recur = recur
        self.prune = prune
        self.default = default
        self.clear = clear
        self.default_toml = {}
        if os.path.exists(DEFAULT_TEXTURE_IMPORT_FILE):
            with open(DEFAULT_TEXTURE_IMPORT_FILE, 'r') as f:
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
    print_info("Success!")


def edit_parameter(node, parameter, options, default, option_type: type = str):
    if parameter not in node:
        node[parameter] = default
    inpt = varinput(f"{parameter} ({node[parameter]}): ")
    if inpt:
        try:
            if option_type == bool:
                inpt = str_to_bool(inpt)
            else:
                inpt = option_type(inpt)
            if inpt in options:
                node[parameter] = inpt
            else:
                raise TypeError()
        except TypeError:
            print_warning(f"Invalid input. Must be one of {options}.")


def edit_image_parameters(node):
    edit_parameter(node, 'storage', ["discard", "keep"], "discard")
    edit_parameter(node, 'min filter',
                   ["nearest", "linear", "nearest mipmap nearest", "nearest mipmap linear",
                    "linear mipmap nearest", "linear mipmap linear"], "nearest")
    edit_parameter(node, 'mag filter', ["nearest", "linear"], "nearest")
    edit_parameter(node, 'generate mipmaps', [True, False], False, bool)


def edit_gif_parameters(node):
    edit_parameter(node, 'storage', ["discard", "keep"], "discard")
    edit_parameter(node, 'min filter',
                   ["nearest", "linear", "nearest mipmap nearest", "nearest mipmap linear",
                    "linear mipmap nearest", "linear mipmap linear"], "nearest")
    edit_parameter(node, 'mag filter', ["nearest", "linear"], "nearest")
    edit_parameter(node, 'generate mipmaps', [True, False], False, bool)


def edit_svg_parameters(node):
    edit_parameter(node, 'abstract storage', ["discard", "keep"], "discard")
    edit_parameter(node, 'image storage', ["discard", "keep"], "discard")
    edit_parameter(node, 'min filter',
                   ["nearest", "linear", "nearest mipmap nearest", "nearest mipmap linear",
                    "linear mipmap nearest", "linear mipmap linear"], "linear")
    edit_parameter(node, 'mag filter', ["nearest", "linear"], "linear")
    edit_parameter(node, 'generate mipmaps', ["off", "auto", "manual"], "off")


def generate_spritesheet():
    filepath = ""
    while len(filepath) == 0:
        filepath = varinput("Texture filepath: ")
    filepath = res_path(filepath)

    if not filepath.endswith(IMPORT_FILE_EXTENSIONS_SPRITESHEETABLE):
        print_error(f"Can only generate spritesheets for images of the following file types: "
                    f"{IMPORT_FILE_EXTENSIONS_SPRITESHEETABLE}")
        return

    tml = {}
    if os.path.exists(filepath + '.oly'):
        with open(filepath + '.oly', 'r') as f:
            tml = toml.load(f)

    def get_texture_index():
        if 'texture' in tml:
            max_texture_index = len(tml['texture'])
        else:
            max_texture_index = 0
            tml['texture'] = []

        if max_texture_index > 0:
            index = ""
            while not index.isdigit():
                if max_texture_index == 1:
                    index = varinput(f"Texture index [0-1] (0): ")
                    if not index:
                        index = "0"
                else:
                    index = varinput(f"Texture index [0-{max_texture_index}]: ")
                if index.isdigit() and (int(index) < 0 or int(index) > max_texture_index):
                    print_error(f"Texture index must be from interval [0-{max_texture_index}]")
            index = int(index)
        else:
            index = 0

        if index == max_texture_index:
            tml['texture'].append({})
        return index

    texture_index = get_texture_index()
    texture_node = tml['texture'][texture_index]
    if filepath.endswith('.svg'):
        edit_svg_parameters(texture_node)
    else:
        edit_image_parameters(texture_node)
    texture_node['anim'] = True

    edit_bound_int_parameter(texture_node, 'rows', 1, inf)
    edit_bound_int_parameter(texture_node, 'cols', 1, inf)
    edit_bound_int_parameter(texture_node, 'delay cs', 0, inf)
    edit_optional_bound_int_parameter(texture_node, 'cell width override', 0, inf)
    edit_optional_bound_int_parameter(texture_node, 'cell height override', 0, inf)
    edit_optional_bool_parameter(texture_node, 'row major')
    edit_optional_bool_parameter(texture_node, 'row up')

    with open(filepath + '.oly', 'w') as f:
        toml.dump(tml, f)
    print_info("Success!")


def edit_defaults():
    while True:
        choice = input("(1) Edit image import\n(2) Edit gif import\n(3) Edit svg import\n(0) Back\n")
        if choice == "0":
            return
        elif choice.isdigit() and 0 < int(choice) <= 3:
            selection = int(choice)
            break
        else:
            print_error("Invalid input.")

    tml = {}
    if os.path.exists(DEFAULT_TEXTURE_IMPORT_FILE):
        with open(DEFAULT_TEXTURE_IMPORT_FILE) as f:
            tml = toml.load(f)

    match selection:
        case 1:
            if 'image' not in tml:
                tml['image'] = {}
            edit_image_parameters(tml['image'])
        case 2:
            if 'gif' not in tml:
                tml['gif'] = {}
            edit_gif_parameters(tml['gif'])
        case 3:
            if 'svg' not in tml:
                tml['svg'] = {}
            edit_svg_parameters(tml['svg'])

    with open(DEFAULT_TEXTURE_IMPORT_FILE, 'w') as f:
        toml.dump(tml, f)
    print_info("Success!")


def reset_defaults():
    while True:
        choice = input("(1) Reset image import\n(2) Reset gif import\n(3) Reset svg import\n(4) Reset all\n(0) Back\n")
        if choice == "0":
            return
        elif choice.isdigit() and 0 < int(choice) <= 4:
            selection = int(choice)
            break
        else:
            print_error("Invalid input.")

    tml = {}
    if os.path.exists(DEFAULT_TEXTURE_IMPORT_FILE):
        with open(DEFAULT_TEXTURE_IMPORT_FILE) as f:
            tml = toml.load(f)

    def reset_image():
        if 'image' not in tml:
            tml['image'] = {}
        tml['image']['storage'] = "discard"
        tml['image']['min filter'] = "nearest"
        tml['image']['mag filter'] = "nearest"
        tml['image']['generate mipmaps'] = False

    def reset_gif():
        if 'gif' not in tml:
            tml['gif'] = {}
        tml['gif']['storage'] = "discard"
        tml['gif']['min filter'] = "nearest"
        tml['gif']['mag filter'] = "nearest"
        tml['gif']['generate mipmaps'] = False

    def reset_svg():
        if 'svg' not in tml:
            tml['svg'] = {}
        tml['svg']['abstract storage'] = "discard"
        tml['svg']['image storage'] = "discard"
        tml['svg']['min filter'] = "linear"
        tml['svg']['mag filter'] = "linear"
        tml['svg']['generate mipmaps'] = "off"

    match selection:
        case 1:
            reset_image()
        case 2:
            reset_gif()
        case 3:
            reset_svg()
        case 4:
            reset_image()
            reset_gif()
            reset_svg()

    with open(DEFAULT_TEXTURE_IMPORT_FILE, 'w') as f:
        toml.dump(tml, f)
    print_info("Success!")


TOOL = ToolNode("textures", "Manipulate texture import (.oly) files.")

IMPORT_TEXTURES = ToolNode("import", "Generate import files.", import_textures)
TOOL.add_child(IMPORT_TEXTURES)

GENERATE_SPRITESHEET = ToolNode("spritesheet", "Generate spritesheet texture in import files.", generate_spritesheet)
TOOL.add_child(GENERATE_SPRITESHEET)

EDIT_DEFAULTS = ToolNode("edit defaults", "Edit the default texture import structure.", edit_defaults)
TOOL.add_child(EDIT_DEFAULTS)

RESET_DEFAULTS = ToolNode("reset defaults", "Reset the default texture import structure.", reset_defaults)
TOOL.add_child(RESET_DEFAULTS)
