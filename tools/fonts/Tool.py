from common import *

IMPORT_FILE_EXTENSIONS = (".ttf", ".otf")


class FontImporter(Importer):
    def __init__(self, folder: str, recur: bool, prune: bool, default: bool, clear: bool):
        super().__init__(folder, recur, prune, default, clear)
        super().setup("fonts", IMPORT_FILE_EXTENSIONS)

    def write_default_toml(self, file: str, tml: dict) -> bool:
        if 'font_face' in tml and not self.default:
            return False
        tml['font_face'] = {}
        tml['font_atlas'] = [{}]

        if 'font face storage' in self.default_toml:
            tml['font_face']['storage'] = self.default_toml['font face storage']
        if 'font atlas storage' in self.default_toml:
            tml['font_atlas'][0]['storage'] = self.default_toml['font atlas storage']

        def write_atlas_option(parameter: str):
            if parameter in self.default_toml:
                tml['font_atlas'][0][parameter] = self.default_toml[parameter]

        write_atlas_option('font size')
        write_atlas_option('mag filter')
        write_atlas_option('min filter')
        write_atlas_option('generate mipmaps')
        write_atlas_option('common buffer preset')
        write_atlas_option('common buffer')
        return True


def import_fonts():
    import_folder("Fonts folder: ", FontImporter)
    print_info("Success!")


def import_fonts_manifest():
    import_manifest("fonts", FontImporter)


def import_fonts_manifest_tool():
    import_manifest("fonts", FontImporter)
    print_info("Success!")


TOOL = ToolNode("fonts", "Manipulate font import files.")

IMPORT_FONTS = ToolNode("import", "Generate import files.", import_fonts)
TOOL.add_child(IMPORT_FONTS)

IMPORT_ALL_FONTS = ToolNode("import all", "Generate import files for fonts manifest", import_fonts_manifest_tool)
TOOL.add_child(IMPORT_ALL_FONTS)
