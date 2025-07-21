import json
import os
import shutil
from enum import Enum

import toml

from . import Ellipse, NGon, Paragraph, PolyComposite, Polygon, Sprite, SpriteAtlas, TileMap, SpriteNonant, Common
from ToolNode import ToolNode, print_info, print_error, varinput

MANIFEST_PATH = 'archetype/manifest.txt'
CACHE_PATH = 'archetype/cache.json'
GEN_PATH = '../gen/archetypes'


# TODO v3 only support SPRITE and TEXT - POLYGON/ELLIPSE are probably only useful for debugging or prototyping, but aren't efficient for rendering in large numbers.
# TODO v3 combine SPRITE and TEXT shaders.
class Batch(Enum):
    SPRITE = 0,
    POLYGON = 1,
    ELLIPSE = 2,
    TEXT = 3


RESERVED_NAMES = [
    "transformer",
    "draw",
    "on_tick",
    "params",
    "frame_format",
    "_method",
    "_layer",
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
        self.sprite_nonants = self.archetype['sprite_nonant'] if 'sprite_nonant' in self.archetype else []

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
        register_batch(self.sprite_nonants, Batch.SPRITE)

    def registry_includes(self) -> str:
        incl = "#include \"registries/Loader.h\"\n"
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
        if len(self.tilemaps) > 0:
            incl += "#include \"registries/graphics/extensions/SpriteNonants.h\"\n"
        return incl

    @staticmethod
    def write_declarations(renderables: [], class_name: str, tabs: int) -> str:
        decl = ""
        for renderable in renderables:
            decl += "\t" * tabs + f"{class_name} {renderable['name']};\n"
        return decl

    def data_members(self) -> str:
        decl = ""
        decl += self.write_declarations(self.sprites, "rendering::SpriteRef", 2)
        decl += self.write_declarations(self.polygons, "rendering::PolygonRef", 2)
        decl += self.write_declarations(self.poly_composites, "rendering::PolyCompositeRef", 2)
        decl += self.write_declarations(self.ngons, "rendering::NGonRef", 2)
        decl += self.write_declarations(self.ellipses, "rendering::EllipseRef", 2)
        decl += self.write_declarations(self.paragraphs, "rendering::ParagraphRef", 2)
        decl += self.write_declarations(self.sprite_atlases, "rendering::SpriteAtlasRef", 2)
        decl += self.write_declarations(self.tilemaps, "rendering::TileMapRef", 2)
        decl += self.write_declarations(self.sprite_nonants, "rendering::SpriteNonantRef", 2)
        return decl

    def initialization(self) -> str:
        c =\
f"""        {{
            reg::params::Transformer2D params;
{Common.write_named_transformer_2d(self.archetype, 'params', 3)}
            transformer = reg::load_transformer_2d(params);
        }}
"""
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
        for sprite_nonant in self.sprite_nonants:
            c += SpriteNonant.constructor(sprite_nonant) + "\n"
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

    @staticmethod
    def write_transformer_attachment(renderables, transformer_accessor="transformer"):
        att = ""
        for renderable in renderables:
            att += f"\n\t\t{renderable['name']}->{transformer_accessor}.attach_parent(&transformer);"
        return att

    def transformer_attachments(self) -> str:
        att = ""
        att += self.write_transformer_attachment(self.sprites)
        att += self.write_transformer_attachment(self.polygons)
        att += self.write_transformer_attachment(self.poly_composites)
        att += self.write_transformer_attachment(self.ngons)
        att += self.write_transformer_attachment(self.ellipses)
        att += self.write_transformer_attachment(self.paragraphs)
        att += self.write_transformer_attachment(self.sprite_atlases, "sprite.transformer")
        att += self.write_transformer_attachment(self.tilemaps, "set_transformer()")
        att += self.write_transformer_attachment(self.sprite_nonants, "set_transformer()")
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
            draw += f"\t\t{renderable}->draw();\n"
        if batch != -1:
            draw += f"\t\tif ({self.batch_flush()})\n"
            draw += "\t" + self.write_render(batch)
        return draw

    def on_tick(self) -> str:
        tick = ""
        for sprite_atlas in self.sprite_atlases:
            tick += f"\t\t{sprite_atlas['name']}->on_tick();\n"
        return tick


def generate_header(proto: Archetype) -> str:
    return f"""#pragma once

#include "Olympian.h"

namespace oly::gen
{{
    struct {proto.name}
    {{
        Transformer2D transformer;
{proto.data_members()}
        {proto.name}();
        {proto.name}(const {proto.name}&) = default;
        {proto.name}({proto.name}&&) = default;
        {proto.name}& operator=(const {proto.name}&) = default;
        {proto.name}& operator=({proto.name}&&) = default;

        const Transform2D& get_local() const {{ return transformer.get_local(); }}
        Transform2D& set_local() {{ return transformer.set_local(); }}
{f"""
        void draw(bool {proto.batch_flush()}) const;
""" if len(proto.draw_list) > 0 else ""}
        void on_tick() const;
    }};
}}
"""


def generate_cpp(proto: Archetype) -> str:
    return f"""#include \"{proto.name}.h\"

{proto.registry_includes()}
namespace oly::gen
{{
    {proto.name}::{proto.name}()
    {{
{proto.initialization()}
{proto.transformer_attachments()}
    }}
{f"""

    void {proto.name}::draw(bool {proto.batch_flush()}) const
    {{
{proto.draw_calls()}\t}}
""" if len(proto.draw_list) > 0 else ""}

    void {proto.name}::on_tick() const
    {{
{proto.on_tick()}\t}}
}}
"""


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
