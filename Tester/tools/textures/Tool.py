import os

from common import *

IMPORT_FILE_EXTENSIONS = (".png", ".jpg", ".jpeg", ".bmp", ".svg", ".gif")
IMPORT_FILE_EXTENSIONS_IMAGES = (".png", ".jpg", ".jpeg", ".bmp")
IMPORT_FILE_EXTENSIONS_SPRITESHEETABLE = (".png", ".jpg", ".jpeg", ".bmp", ".svg")
DEFAULT_TEXTURE_IMPORT_FILE = "textures/default.toml"


class TextureImporter(Importer):
    def __init__(self, folder: str, recur: bool, prune: bool, default: bool, clear: bool):
        super().__init__(folder, recur, prune, default, clear)
        super().setup("textures", IMPORT_FILE_EXTENSIONS)

    def write_default_toml(self, file: str, tml: dict) -> bool:
        if 'texture' in tml and not self.default:
            return False

        tml['texture'] = [{}]
        for k, v in self.default_toml[self.default_header(file)].items():
            tml['texture'][0][k] = v
        return True

    @staticmethod
    def default_header(file: str):
        if file.endswith(IMPORT_FILE_EXTENSIONS_IMAGES):
            return 'image'
        elif file.endswith(".svg"):
            return 'svg'
        elif file.endswith(".gif"):
            return 'gif'


def import_textures():
    import_folder("Textures folder: ", TextureImporter)
    print_info("Success!")


def import_textures_manifest():
    import_manifest("textures", TextureImporter)


def import_textures_manifest_tool():
    import_manifest("textures", TextureImporter)
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


TOOL = ToolNode("textures", "Manipulate texture import files.")

IMPORT_TEXTURES = ToolNode("import", "Generate import files.", import_textures)
TOOL.add_child(IMPORT_TEXTURES)

IMPORT_ALL_TEXTURES = ToolNode("import all", "Generate import files for texture manifest", import_textures_manifest_tool)
TOOL.add_child(IMPORT_ALL_TEXTURES)

GENERATE_SPRITESHEET = ToolNode("spritesheet", "Generate spritesheet texture in import files.", generate_spritesheet)
TOOL.add_child(GENERATE_SPRITESHEET)

EDIT_DEFAULTS = ToolNode("edit defaults", "Edit the default texture import structure.", edit_defaults)
TOOL.add_child(EDIT_DEFAULTS)

RESET_DEFAULTS = ToolNode("reset defaults", "Reset the default texture import structure.", reset_defaults)
TOOL.add_child(RESET_DEFAULTS)
