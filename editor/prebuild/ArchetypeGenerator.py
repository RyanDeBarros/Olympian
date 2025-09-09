import json
import os
import shutil
from pathlib import Path

import toml

from assets import *
from editor.tools.TOMLAdapter import meta

ARCHETYPE_GEN_PATH = Path('.gen/archetypes').resolve()

# TODO v4 only support SPRITE and TEXT - POLYGON/ELLIPSE are probably only useful for debugging or prototyping, but aren't efficient for rendering in large numbers.
# TODO v4 combine SPRITE and TEXT shaders.


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

		self.variable_list = set()

		def register_batch(renderables):
			for renderable in renderables:
				name = renderable['name']
				assert name not in RESERVED_NAMES, f"'{name}' is a reserved name"
				assert len(name) > 0 and (name[0].isalpha() or name[0] == "_") and not any(
					c.isspace() for c in name), f"Invalid variable name '{name}'"
				assert name not in self.variable_list, f"Repeated variable name '{name}'"
				self.variable_list.add(name)

		register_batch(self.sprites)
		register_batch(self.polygons)
		register_batch(self.poly_composites)
		register_batch(self.ngons)
		register_batch(self.ellipses)
		register_batch(self.paragraphs)
		register_batch(self.sprite_atlases)
		register_batch(self.tilemaps)
		register_batch(self.sprite_nonants)

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
	def write_declarations(renderables: list, class_name: str, tabs: int) -> str:
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
		c = \
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

	def draw_calls(self) -> str:
		draw = ""
		for renderable in self.draw_list:
			draw += f"\t\t{renderable}->draw();\n"
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
		void draw() const;
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
	void {proto.name}::draw() const
	{{
{proto.draw_calls()}\t}}
""" if len(proto.draw_list) > 0 else ""}
	void {proto.name}::on_tick() const
	{{
{proto.on_tick()}\t}}
}}
"""


# TODO v4 move Cache and manifest generator into different file from source code generator above
class Cache:
	def __init__(self):
		self.cache_path = Path('.gen/cache.json').resolve()
		self.cache: dict[str, float] = {}
		if self.cache_path.exists():
			with open(self.cache_path, 'r') as f:
				try:
					self.cache: dict = json.load(f)
				except json.JSONDecodeError:
					pass
		self.marked: list[str] = []

	def dump(self):
		self.cache = {file: self.cache[file] for file in self.marked}
		self.marked.clear()
		with open(self.cache_path, 'w') as f:
			json.dump(self.cache, f)

	def is_dirty(self, file: Path) -> bool:
		return file.as_posix() not in self.cache or self.cache[file.as_posix()] != file.stat().st_mtime

	def update(self, file: Path):
		self.cache[file.as_posix()] = file.stat().st_mtime

	def mark(self, file: Path):
		self.marked.append(file.as_posix())

	def clear(self):
		self.cache.clear()
		self.marked.clear()
		self.dump()
		shutil.rmtree(ARCHETYPE_GEN_PATH)
		os.makedirs(ARCHETYPE_GEN_PATH)


def generate(asset_filepath: Path):
	with open(asset_filepath, 'r') as f:
		tml = toml.load(f)
		if 'archetype' not in tml:
			return
		proto = Archetype(tml)

	gen_folder = os.path.join(ARCHETYPE_GEN_PATH, proto.gen_folder)
	hdr = generate_header(proto)
	cpp = generate_cpp(proto)

	os.makedirs(gen_folder, exist_ok=True)
	with open(os.path.join(gen_folder, proto.name + ".h"), 'w') as f:
		f.write(hdr)
	with open(os.path.join(gen_folder, proto.name + ".cpp"), 'w') as f:
		f.write(cpp)


def generate_manifest():
	def generate_file(file: Path):
		if file.suffix == ".toml":
			m = meta(file)
			if 'type' in m and m['type'] == 'archetype':
				cache.mark(file)
				if cache.is_dirty(file):
					generate(file)
					cache.update(file)

	def generate_folder(folder):
		for file in Path(folder).rglob("*.toml"):
			generate_file(file)

	assets = Path('.gen/manifest.txt').read_text().splitlines()
	cache = Cache()
	for asset in assets:
		asset_path = Path(f"res/{asset}")
		if asset_path.is_file():
			generate_file(asset_path)
		elif asset_path.is_dir():
			generate_folder(asset_path)
	cache.dump()
