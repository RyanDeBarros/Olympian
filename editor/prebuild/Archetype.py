from pathlib import Path

from editor.tools import TOMLAdapter
from .assets import *

# TODO v4 archetypes should not be POD containers? Could inherit from Archetype class or something. A KinematicBody archetype would for example inherit from KinematicBody.
# TODO v4 generating manifest should load a giant dictionary of names to archetypes before processing any specific ones, so that referenced sub-archetypes don't need to be re-loaded from file. If there's a duplicate name, generate a build error. Within archetypes, reference by name not by file path.


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
	def __init__(self, name, toml):
		self.name = name
		self.archetype = toml['archetype']
		self.sprites = self.archetype['sprite'] if 'sprite' in self.archetype else []
		self.paragraphs = self.archetype['paragraph'] if 'paragraph' in self.archetype else []
		self.sprite_atlases = self.archetype['sprite_atlas'] if 'sprite_atlas' in self.archetype else []
		self.tilemaps = self.archetype['tilemap'] if 'tilemap' in self.archetype else []
		self.sprite_nonants = self.archetype['sprite_nonant'] if 'sprite_nonant' in self.archetype else []

		assert len(self.name) > 0 and self.name[0].isalpha(), "Invalid archetype name"
		self.draw_list = self.archetype['draw'] if 'draw' in self.archetype else []

		self.variable_list = set()

		def register_batch(renderables):
			for renderable in renderables:
				# noinspection PyShadowingNames
				name = renderable['name']
				assert name not in RESERVED_NAMES, f"'{name}' is a reserved name"
				assert len(name) > 0 and (name[0].isalpha() or name[0] == "_") and not any(
					c.isspace() for c in name), f"Invalid variable name '{name}'"
				assert name not in self.variable_list, f"Repeated variable name '{name}'"
				self.variable_list.add(name)

		register_batch(self.sprites)
		register_batch(self.paragraphs)
		register_batch(self.sprite_atlases)
		register_batch(self.tilemaps)
		register_batch(self.sprite_nonants)

	def registry_includes(self) -> str:
		incl = "#include \"registries/Loader.h\"\n"
		if len(self.sprites) > 0:
			incl += "#include \"registries/graphics/sprites/Sprites.h\"\n"
		if len(self.paragraphs) > 0:
			incl += "#include \"registries/graphics/text/Paragraphs.h\"\n"
		if len(self.sprite_atlases) > 0:
			incl += "#include \"registries/graphics/sprites/SpriteAtlases.h\"\n"
		if len(self.tilemaps) > 0:
			incl += "#include \"registries/graphics/sprites/TileMaps.h\"\n"
		if len(self.tilemaps) > 0:
			incl += "#include \"registries/graphics/sprites/SpriteNonants.h\"\n"
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
		for paragraph in self.paragraphs:
			c += Paragraph.constructor(paragraph) + "\n"
		for sprite_atlas in self.sprite_atlases:
			c += SpriteAtlas.constructor(sprite_atlas) + "\n"
		for tilemap in self.tilemaps:
			c += TileMap.constructor(tilemap) + "\n"
		for sprite_nonant in self.sprite_nonants:
			c += SpriteNonant.constructor(sprite_nonant) + "\n"
		return c[:-1] if len(c) > 0 else ""  # don't keep last \n

	@staticmethod
	def write_transformer_attachment(renderables, transformer_accessor="transformer"):
		att = ""
		for renderable in renderables:
			att += f"\n\t\t{renderable['name']}->{transformer_accessor}.attach_parent(&transformer);"
		return att

	def transformer_attachments(self) -> str:
		att = ""
		att += self.write_transformer_attachment(self.sprites)
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

#include <Olympian.h>

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


def generate_archetype(asset_filepath: Path, gen_root: Path, res_folder: Path):
	if TOMLAdapter.meta(asset_filepath).get('type') != 'archetype':
		return
	toml = TOMLAdapter.load(asset_filepath)
	if 'archetype' not in toml:
		return
	proto = Archetype(asset_filepath.stem, toml)

	gen_folder = gen_root / asset_filepath.relative_to(res_folder.resolve()).parent
	gen_folder.mkdir(parents=True, exist_ok=True)
	(gen_folder / f"{proto.name}.h").write_text(generate_header(proto))
	(gen_folder / f"{proto.name}.cpp").write_text(generate_cpp(proto))
