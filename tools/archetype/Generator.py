import argparse
import os
from enum import Enum

import toml

import Ellipse
import NGon
import Paragraph
import PolyComposite
import Polygon
import Sprite
import SpriteAtlas
import TileMap


class Batch(Enum):
    SPRITE = 0,
    POLYGON = 1,
    ELLIPSE = 2,
    TEXT = 3


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
        assert len(self.name) > 0 and self.name[0].isalpha(), "Invalid archetype name"
        self.draw_list = self.archetype['draw']

        self.batch_map = {}

        def register_batch(renderables, batch: Batch):
            for renderable in renderables:
                assert renderable['name'] not in self.batch_map, "Repeated variable name"
                self.batch_map[renderable['name']] = batch

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
        decl += self.write_declarations(self.ngons, "Ngon", "rendering", 2)
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
        decl += self.write_declarations(self.sprite_atlases, "SpriteAtlasExtension", "reg::params", 3)
        decl += self.write_declarations(self.tilemaps, "TileMap", "reg::params", 3)
        return decl

    def constructors(self) -> str:
        c = ""
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

    @staticmethod
    def write_initializer(renderables, load):
        ini = ""
        for renderable in renderables:
            ini += f"\t\t{renderable['name']}(reg::{load}(c.{renderable['name']})),\n"
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
        return ini[:-2] if len(ini) > 0 else ""  # don't keep last ,\n

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
        # TODO
        return tick


def generate_header(proto: Archetype) -> str:
    hdr = """#pragma once

#include "Olympian.h"

"""

    hdr += proto.includes()

    hdr += f"""
namespace oly::gen
{{
    struct {proto.name}
    {{
"""

    hdr += proto.declarations()

    hdr += """
    private:
        struct Constructor
        {
"""

    hdr += proto.constructor_declarations()

    hdr += f"""
            Constructor();
        }};

    public:
        {proto.name}(Constructor = {{}});

        void draw(bool {proto.batch_flush()}) const;

        void on_tick() const;
    }};
}}
"""
    return hdr


def generate_cpp(proto: Archetype) -> str:
    cpp = f"""#include \"{proto.name}.h\"

namespace oly::gen
{{
    {proto.name}::Constructor::Constructor()
    {{
"""

    cpp += proto.constructors()

    cpp += f"""    }}
    
    {proto.name}::{proto.name}(Constructor c) :
"""

    cpp += proto.initializer_list()

    cpp += f"""
    {{}}
    
    void {proto.name}::draw(bool {proto.batch_flush()}) const
    {{
"""

    cpp += proto.draw_calls()

    cpp += f"""    }}
    
    void {proto.name}::on_tick() const
    {{
"""

    cpp += proto.on_tick()

    cpp += """    }
}
"""

    return cpp


def generate(asset_filepath: str, gen_folder: str):
    with open(asset_filepath, 'r') as f:
        proto = Archetype(toml.load(f))

    hdr = generate_header(proto)
    cpp = generate_cpp(proto)

    os.makedirs(gen_folder, exist_ok=True)
    with open(os.path.join(gen_folder, proto.name + ".h"), 'w') as f:
        f.write(hdr)
    with open(os.path.join(gen_folder, proto.name + ".cpp"), 'w') as f:
        f.write(cpp)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Generates archetype source code from archetype asset file.")

    parser.add_argument('-a', '--asset', required=True, type=str, help="Archetype asset to generate")
    parser.add_argument('-g', '--gen_folder', required=True, type=str, help="Folder to generate source code")

    args = parser.parse_args()
    generate(args.asset, args.gen_folder)
