import json
import os
import shutil
from enum import Enum

import toml

from . import Ellipse, NGon, Paragraph, PolyComposite, Polygon, Sprite, SpriteAtlas, TileMap, Common
from Tool import ToolNode, print_info, print_error, varinput

MANIFEST_PATH = 'archetype/manifest.txt'
CACHE_PATH = 'archetype/cache.json'
GEN_PATH = '../gen/archetypes'


class Batch(Enum):
    SPRITE = 0,
    POLYGON = 1,
    ELLIPSE = 2,
    TEXT = 3


RESERVED_NAMES = [
    "transformer",
    "draw",
    "on_tick",
    "free_constructor"
]


class Archetype:
    def __init__(self, tml):
        self.archetype = tml['archetype']
        self.sprites = self.archetype['sprite'] if 'sprite' in self.archetype else []
        self.polygons = self.archetype['polygon'] if 'polygon' in self.archetype else []
        self.poly_composites = self.archetype['poly_composite'] if 'poly_composite' in self.archetype else []
        self.ngons = self.archetype['ngon'] if 'ngon' in self.archetype else []
        self.ellipses = self.archetype['ellipse'] if 'ellipse' in self.archetype else []
        self.paragraphs = self.archetype['paragraph'] if 'paragraph' in self.archetype else []
        self.sprite_atlases = self.archetype['sprite_atlas'] if 'sprite_atlas' in self.archetype else []
        self.tilemaps = self.archetype['tilemap'] if 'tilemap' in self.archetype else []

        self.name: str = self.archetype['name']
        self.gen_folder = ""
        if 'gen folder' in self.archetype:
            self.gen_folder = self.archetype['gen folder']
        assert len(self.name) > 0 and self.name[0].isalpha(), "Invalid archetype name"
        self.draw_list = self.archetype['draw'] if 'draw' in self.archetype else []
        self.singleton: bool = 'singleton' in self.archetype and self.archetype['singleton']

        self.batch_map = {}

        def register_batch(renderables, batch: Batch):
            for renderable in renderables:
                name = renderable['name']
                assert name not in RESERVED_NAMES, f"'{name}' is a reserved name"
                assert len(name) > 0 and (name[0].isalpha() or name[0] == "_") and not any(
                    c.isspace() for c in name), f"Invalid variable name '{name}'"
                assert name not in self.batch_map, f"Repeated variable name '{name}'"
                self.batch_map[name] = batch

        register_batch(self.sprites, Batch.SPRITE)
        register_batch(self.polygons, Batch.POLYGON)
        register_batch(self.poly_composites, Batch.POLYGON)
        register_batch(self.ngons, Batch.POLYGON)
        register_batch(self.ellipses, Batch.ELLIPSE)
        register_batch(self.paragraphs, Batch.TEXT)
        register_batch(self.sprite_atlases, Batch.SPRITE)
        register_batch(self.tilemaps, Batch.SPRITE)

    def includes(self) -> str:
        incl = ""
        if len(self.sprites) > 0:
            incl += "#include \"registries/graphics/primitives/Sprites.h\"\n"
        if len(self.polygons) > 0 or len(self.poly_composites) > 0 or len(self.ngons) > 0:
            incl += "#include \"registries/graphics/primitives/Polygons.h\"\n"
        if len(self.ellipses) > 0:
            incl += "#include \"registries/graphics/primitives/Ellipses.h\"\n"
        if len(self.paragraphs) > 0:
            incl += "#include \"registries/graphics/text/Paragraphs.h\"\n"
        if len(self.sprite_atlases) > 0:
            incl += "#include \"registries/graphics/extensions/SpriteAtlases.h\"\n"
        if len(self.tilemaps) > 0:
            incl += "#include \"registries/graphics/extensions/TileMaps.h\"\n"
        return incl

    @staticmethod
    def write_declarations(renderables: [], class_name: str, prefix: str, tabs: int) -> str:
        decl = ""
        for renderable in renderables:
            decl += "\t" * tabs + f"{prefix}::{class_name} {renderable['name']};\n"
        return decl

    def declarations(self) -> str:
        decl = ""
        decl += self.write_declarations(self.sprites, "Sprite", "rendering", 2)
        decl += self.write_declarations(self.polygons, "Polygon", "rendering", 2)
        decl += self.write_declarations(self.poly_composites, "PolyComposite", "rendering", 2)
        decl += self.write_declarations(self.ngons, "NGon", "rendering", 2)
        decl += self.write_declarations(self.ellipses, "Ellipse", "rendering", 2)
        decl += self.write_declarations(self.paragraphs, "Paragraph", "rendering", 2)
        decl += self.write_declarations(self.sprite_atlases, "SpriteAtlasExtension", "rendering", 2)
        decl += self.write_declarations(self.tilemaps, "TileMap", "rendering", 2)
        return decl

    def constructor_declarations(self) -> str:
        decl = ""
        decl += self.write_declarations(self.sprites, "Sprite", "reg::params", 3)
        decl += self.write_declarations(self.polygons, "Polygon", "reg::params", 3)
        decl += self.write_declarations(self.poly_composites, "PolyComposite", "reg::params", 3)
        decl += self.write_declarations(self.ngons, "NGon", "reg::params", 3)
        decl += self.write_declarations(self.ellipses, "Ellipse", "reg::params", 3)
        decl += self.write_declarations(self.paragraphs, "Paragraph", "reg::params", 3)
        decl += self.write_declarations(self.sprite_atlases, "SpriteAtlas", "reg::params", 3)
        decl += self.write_declarations(self.tilemaps, "TileMap", "reg::params", 3)
        return decl

    def constructors(self) -> str:
        c = Common.write_named_transformer_2d(self.archetype, 'transformer')
        for sprite in self.sprites:
            c += Sprite.constructor(sprite) + "\n"
        for polygon in self.polygons:
            c += Polygon.constructor(polygon) + "\n"
        for poly_composite in self.poly_composites:
            c += PolyComposite.constructor(poly_composite) + "\n"
        for ngon in self.ngons:
            c += NGon.constructor(ngon) + "\n"
        for ellipse in self.ellipses:
            c += Ellipse.constructor(ellipse) + "\n"
        for paragraph in self.paragraphs:
            c += Paragraph.constructor(paragraph) + "\n"
        for sprite_atlas in self.sprite_atlases:
            c += SpriteAtlas.constructor(sprite_atlas) + "\n"
        for tilemap in self.tilemaps:
            c += TileMap.constructor(tilemap) + "\n"
        return c[:-1] if len(c) > 0 else ""  # don't keep last \n

    def write_initializer(self, renderables, load):
        ini = ""
        if self.singleton:
            for renderable in renderables:
                ini += f"\t\t{renderable['name']}(reg::{load}(std::move(constructor().{renderable['name']}))),\n"
        else:
            for renderable in renderables:
                ini += f"\t\t{renderable['name']}(reg::{load}(constructor().{renderable['name']})),\n"
        return ini

    def initializer_list(self) -> str:
        ini = ""
        ini += self.write_initializer(self.sprites, "load_sprite")
        ini += self.write_initializer(self.polygons, "load_polygon")
        ini += self.write_initializer(self.poly_composites, "load_poly_composite")
        ini += self.write_initializer(self.ngons, "load_ngon")
        ini += self.write_initializer(self.ellipses, "load_ellipse")
        ini += self.write_initializer(self.paragraphs, "load_paragraph")
        ini += self.write_initializer(self.sprite_atlases, "load_sprite_atlas")
        ini += self.write_initializer(self.tilemaps, "load_tilemap")
        return ini[:-1] if len(ini) > 0 else ""  # don't keep last \n

    @staticmethod
    def write_transformer_attachment(renderables):
        att = ""
        for renderable in renderables:
            att += f"\t\t{renderable['name']}.transformer.attach_parent(&transformer);\n"
        return att

    def transformer_attachments(self) -> str:
        att = ""
        att += self.write_transformer_attachment(self.sprites)
        att += self.write_transformer_attachment(self.polygons)
        att += self.write_transformer_attachment(self.poly_composites)
        att += self.write_transformer_attachment(self.ngons)
        att += self.write_transformer_attachment(self.ellipses)
        att += self.write_transformer_attachment(self.paragraphs)
        att += self.write_transformer_attachment(self.tilemaps)
        for renderable in self.sprite_atlases:
            att += f"\t\t{renderable['name']}.sprite.transformer.attach_parent(&transformer);\n"
        return att

    @staticmethod
    def write_render(batch) -> str:
        match batch:
            case Batch.SPRITE:
                return "\t\tcontext::render_sprites();\n"
            case Batch.POLYGON:
                return "\t\tcontext::render_polygons();\n"
            case Batch.ELLIPSE:
                return "\t\tcontext::render_ellipses();\n"
            case Batch.TEXT:
                return "\t\tcontext::render_text();\n"

    def batch_flush(self) -> str:
        match self.batch_map[self.draw_list[-1]]:
            case Batch.SPRITE:
                return "flush_sprites"
            case Batch.POLYGON:
                return "flush_polygons"
            case Batch.ELLIPSE:
                return "flush_ellipses"
            case Batch.TEXT:
                return "flush_text"

    def draw_calls(self) -> str:
        draw = ""
        batch = -1
        for renderable in self.draw_list:
            rb = self.batch_map[renderable]
            if rb != batch and batch != -1:
                draw += self.write_render(batch)
            batch = rb
            draw += f"\t\t{renderable}.draw();\n"
        if batch != -1:
            draw += f"\t\tif ({self.batch_flush()})\n"
            draw += "\t" + self.write_render(batch)
        return draw

    def on_tick(self) -> str:
        tick = ""
        for sprite_atlas in self.sprite_atlases:
            tick += f"\t\t{sprite_atlas['name']}.on_tick();\n"
        return tick


def generate_header(proto: Archetype) -> str:
    hdr = """#pragma once

#include "Olympian.h"

"""

    hdr += proto.includes()

    hdr += f"""
namespace oly::gen
{{
\tstruct {proto.name}
\t{{
\t\tstatic void free_constructor();

\t\tTransformer2D transformer;
\t\tconst Transform2D& get_local() const {{ return transformer.get_local(); }}
\t\tTransform2D& set_local() {{ return transformer.set_local(); }}

"""

    hdr += proto.declarations()

    hdr += f"""
\t\t{proto.name}();
\t\t{proto.name}(const {proto.name}&) = default;
\t\t{proto.name}({proto.name}&&) = default;
\t\t{proto.name}& operator=(const {proto.name}&) = default;
\t\t{proto.name}& operator=({proto.name}&&) = default;

\t\tvoid draw({f"bool {proto.batch_flush()}" if len(proto.draw_list) > 0 else ""}) const;

\t\tvoid on_tick() const;
\t}};
}}
"""
    return hdr


def generate_cpp(proto: Archetype) -> str:
    cpp = f"""#include \"{proto.name}.h\"

namespace oly::gen
{{"""

    cpp += """
\tnamespace
\t{
\t\tstruct Constructor
\t\t{
\t\t\tstruct
\t\t\t{
\t\t\t\tTransform2D local;
\t\t\t\tstd::unique_ptr<TransformModifier2D> modifier;
\t\t\t} transformer;
"""

    cpp += proto.constructor_declarations()

    cpp += f"""
\t\t\tConstructor();
\t\t}};

\t\tConstructor::Constructor()
\t\t{{
"""
    cpp += proto.constructors()

    cpp += f"""\t\t}}

\t\tstatic std::unique_ptr<Constructor> _c;
\t\tstatic Constructor& constructor()
\t\t{{
\t\t\tif (!_c)
\t\t\t\t_c = std::make_unique<Constructor>();
\t\t\treturn *_c;
\t\t}}
\t}}

\tvoid {proto.name}::free_constructor()
\t{{
\t\t_c.reset();
\t}}

\t{proto.name}::{proto.name}() :
"""

    cpp += proto.initializer_list()

    cpp += f"""
\t\ttransformer(constructor().transformer.local, std::make_unique<TransformModifier2D>(*constructor().transformer.modifier))
\t{{\n"""

    cpp += proto.transformer_attachments()

    if proto.singleton:
        cpp += "\t\tfree_constructor();\n"

    cpp += f"""\t}}

\tvoid {proto.name}::draw({f"bool {proto.batch_flush()}" if len(proto.draw_list) > 0 else ""}) const
\t{{
"""

    cpp += proto.draw_calls()

    cpp += f"""\t}}

\tvoid {proto.name}::on_tick() const
\t{{
"""

    cpp += proto.on_tick()

    cpp += """\t}
}
"""

    return cpp


class Cache:
    def __init__(self):
        self.cache = {}
        if os.path.exists(CACHE_PATH):
            with open(CACHE_PATH, 'r') as f:
                try:
                    self.cache: dict = json.load(f)
                except json.JSONDecodeError:
                    pass
        self.marked = []

    def dump(self):
        self.cache = {file: self.cache[file] for file in self.marked}
        self.marked.clear()
        with open(CACHE_PATH, 'w') as f:
            json.dump(self.cache, f)

    def is_dirty(self, file: str) -> bool:
        return file not in self.cache or self.cache[file] != os.path.getmtime(file)

    def update(self, file: str):
        self.cache[file] = os.path.getmtime(file)

    def mark(self, file: str):
        self.marked.append(file)

    def clear(self):
        self.cache.clear()
        self.marked.clear()
        self.dump()
        shutil.rmtree(GEN_PATH)
        os.makedirs(GEN_PATH)


def generate(asset_filepath: str):
    with open(asset_filepath, 'r') as f:
        tml = toml.load(f)
        if 'archetype' not in tml:
            return
        proto = Archetype(tml)

    gen_folder = os.path.join(GEN_PATH, proto.gen_folder)
    hdr = generate_header(proto)
    cpp = generate_cpp(proto)

    os.makedirs(gen_folder, exist_ok=True)
    with open(os.path.join(gen_folder, proto.name + ".h"), 'w') as f:
        f.write(hdr)
    with open(os.path.join(gen_folder, proto.name + ".cpp"), 'w') as f:
        f.write(cpp)


def read_manifest_folders():
    return open(MANIFEST_PATH, 'r').read().splitlines()


def generate_manifest():
    asset_folders = read_manifest_folders()

    def generate_folder(folder):
        with os.scandir(os.path.join('../res', folder)) as entries:
            for entry in entries:
                if entry.is_file():
                    cache.mark(entry.path)
                    if cache.is_dirty(entry.path):
                        generate(entry.path)
                        cache.update(entry.path)
                elif entry.is_dir():
                    generate_folder(entry)

    cache = Cache()
    for asset_folder in asset_folders:
        generate_folder(asset_folder)
    cache.dump()


def generate_manifest_tool():
    generate_manifest()
    print_info("Success!")


def view_manifest():
    folders = read_manifest_folders()
    if len(folders) > 0:
        for folder in folders:
            print_info(folder)
    else:
        print_info("Manifest is empty.")


def append_to_manifest():
    folders = read_manifest_folders()
    folder = varinput("Append folder: ").strip()
    if folder in folders:
        print_info(f"'{folder}' already exists in manifest.")
    else:
        with open(MANIFEST_PATH, 'a') as f:
            f.write(folder + '\n')
        print_info(f"Successfully appended '{folder}'.")


def remove_from_manifest():
    folders = read_manifest_folders()
    folder = varinput("Remove folder: ").strip()
    if folder not in folders:
        print_info(f"'{folder}' does not exist in manifest.")
    else:
        folders.remove(folder)
        with open(MANIFEST_PATH, 'w') as f:
            for fd in folders:
                f.write(fd + '\n')
        print_info(f"Successfully removed '{folder}'.")


def is_subfolder(sub, parent):
    return os.path.normpath(parent) == os.path.normpath(os.path.commonpath([sub, parent]))


def remove_recursive_from_manifest():
    folders = read_manifest_folders()
    parent = varinput("Remove folder: ").strip()
    remaining = []
    for folder in folders:
        if not is_subfolder(folder, parent):
            remaining.append(folder)
    with open(MANIFEST_PATH, 'w') as f:
        for fd in remaining:
            f.write(fd + '\n')
    print_info("Success!")


def clear_manifest():
    with open(MANIFEST_PATH, 'w') as f:
        f.write('')
    print_info("Success!")


def edit_manifest():
    folders = read_manifest_folders()
    folder = varinput("Edit folder: ").strip()
    if folder not in folders:
        print_info(f"{folder} does not exist in manifest.")
    else:
        old_folder = folder
        folder = varinput("New folder: ").strip()
        if folder in folders:
            print_error(f"Fail - '{folder}' already exists in manifest.")
        else:
            folders[folders.index(old_folder)] = folder
            with open(MANIFEST_PATH, 'w') as f:
                for fd in folders:
                    f.write(fd + '\n')
            print_info(f"'{old_folder}' -> '{folder}'")


def clean_manifest():
    folders = read_manifest_folders()
    cleaned = []
    for folder in folders:
        if len(folder) == 0:
            continue
        orphan = True
        for possible_parent in folders:
            if len(possible_parent) > 0 and folder != possible_parent and is_subfolder(folder, possible_parent):
                orphan = False
                break
        if orphan:
            cleaned.append(folder)

    with open(MANIFEST_PATH, 'w') as f:
        for fd in cleaned:
            f.write(fd + '\n')
    print_info("Success!")


def clear_cache():
    Cache().clear()
    print_info("Success!")


TOOL = ToolNode("archetype", "Manipulate archetype source code generation.")

GENERATE_MANIFEST = ToolNode("generate",
                             "Generate all archetype source code from assets found in the folders listed in "
                             "tools/archetype/manifest.txt.",
                             generate_manifest_tool)
TOOL.add_child(GENERATE_MANIFEST)

MANIFEST = ToolNode("manifest", "Manipulate tools/archetype/manifest.txt.")
VIEW_MANIFEST = ToolNode("view", "View the manifest file.", view_manifest)
MANIFEST.add_child(VIEW_MANIFEST)
APPEND_TO_MANIFEST = ToolNode("append", "Append folder to manifest file.", append_to_manifest)
MANIFEST.add_child(APPEND_TO_MANIFEST)
REMOVE_FROM_MANIFEST = ToolNode("remove", "Remove folder from manifest file.", remove_from_manifest)
MANIFEST.add_child(REMOVE_FROM_MANIFEST)
REMOVE_RECURSIVE_FROM_MANIFEST = ToolNode("remove recursive", "Remove folder and any subfolders from manifest file.",
                                          remove_recursive_from_manifest)
MANIFEST.add_child(REMOVE_RECURSIVE_FROM_MANIFEST)
CLEAR_MANIFEST = ToolNode("clear", "Clear the manifest file.", clear_manifest)
MANIFEST.add_child(CLEAR_MANIFEST)
EDIT_MANIFEST = ToolNode("edit", "Edit a folder in the manifest file.", edit_manifest)
MANIFEST.add_child(EDIT_MANIFEST)
CLEAN_MANIFEST = ToolNode("clean",
                          "Remove redundant folders from manifest file, such as folders that have parent folders "
                          "already in manifest file.",
                          clean_manifest)
MANIFEST.add_child(CLEAN_MANIFEST)
TOOL.add_child(MANIFEST)

CLEAR_CACHE = ToolNode("clear cache",
                       "Clear the pre-build cache completely, as well as all generated source code in gen/archetypes.",
                       clear_cache)
TOOL.add_child(CLEAR_CACHE)
