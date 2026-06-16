#include "TileMap.h"

#include "core/context/rendering/Textures.h"
#include "core/context/rendering/Sprites.h"
#include "core/context/rendering/Tilesets.h"
#include "core/util/Loader.h"
#include "core/util/Parser.h"

#include "definitions/Keys.h"

namespace oly::rendering
{
	TileMapLayer::TileMapLayer()
		: Super(context::sprite_batch()->weak_from_this())
	{
	}

	TileMapLayer::TileMapLayer(Unbatched)
	{
	}

	TileMapLayer::TileMapLayer(SpriteBatch& batch)
		: Super(batch->weak_from_this())
	{
	}

	void TileMapLayer::draw() const
	{
		for (const auto& [_, sprite] : sprite_map)
			sprite.draw();
	}

	void TileMapLayer::set_camera_invariant(bool is_camera_invariant) const
	{
		for (const auto& [_, sprite] : sprite_map)
			sprite.set_camera_invariant(is_camera_invariant);
	}

	void TileMapLayer::set_batch(Unbatched)
	{
		reset();
		for (auto& [_, sprite] : sprite_map)
			sprite.set_batch(UNBATCHED);
	}

	void TileMapLayer::set_batch(SpriteBatch& batch)
	{
		reset(*batch);
		for (auto& [_, sprite] : sprite_map)
			sprite.set_batch(batch);
	}

	void TileMapLayer::paint_tile(glm::ivec2 tile)
	{
		if (!sprite_map.count(tile))
		{
			if (auto batch = lock())
			{
				auto& sprite = sprite_map.emplace(tile, *batch).first->second;
				sprite.transformer.attach_parent(&transformer);
				update_configuration(tile);
				update_neighbour_configurations(tile);
			}
		}
	}

	void TileMapLayer::unpaint_tile(glm::ivec2 tile)
	{
		auto it = sprite_map.find(tile);
		if (it != sprite_map.end())
		{
			sprite_map.erase(it);
			update_neighbour_configurations(tile);
		}
	}

	void TileMapLayer::update_neighbour_configurations(glm::ivec2 center)
	{
		update_configuration(center + glm::ivec2{  1,  0 });
		update_configuration(center + glm::ivec2{  0,  1 });
		update_configuration(center + glm::ivec2{ -1,  0 });
		update_configuration(center + glm::ivec2{  0, -1 });
		update_configuration(center + glm::ivec2{  1,  1 });
		update_configuration(center + glm::ivec2{ -1,  1 });
		update_configuration(center + glm::ivec2{ -1, -1 });
		update_configuration(center + glm::ivec2{  1, -1 });
	}

	void TileMapLayer::update_configuration(glm::ivec2 tile)
	{
		auto it = sprite_map.find(tile);
		if (it == sprite_map.end())
			return;

		Sprite& sprite = it->second;

		detail::TileConfigGrid painted_tile{};
		for (int y = detail::GridCoordinate::Top; y <= detail::GridCoordinate::Bottom; ++y)
			for (int x = detail::GridCoordinate::Left; x <= detail::GridCoordinate::Right; ++x)
				painted_tile[y][x] = sprite_map.count(tile + glm::ivec2{ 1 - y, x - 1 });

		auto tile_assignment = tileset->get_tile_assignment(painted_tile);

		sprite.set_texture(tile_assignment.desc.file, tile_assignment.desc.file_index);
		sprite.set_tex_coords(tile_assignment.desc.uvs);
		sprite.set_local().position = glm::vec2(tile);
		
		if (static_cast<bool>(tile_assignment.transformation.reflection & detail::TileReflection::X))
			sprite.set_local().scale.x = -glm::abs(sprite.get_local().scale.x);
		else
			sprite.set_local().scale.x = glm::abs(sprite.get_local().scale.x);

		if (static_cast<bool>(tile_assignment.transformation.reflection & detail::TileReflection::Y))
			sprite.set_local().scale.y = -glm::abs(sprite.get_local().scale.y);
		else
			sprite.set_local().scale.y = glm::abs(sprite.get_local().scale.y);
		
		switch (tile_assignment.transformation.rotation)
		{
		case detail::TileRotation::By90:
			sprite.set_local().rotation = glm::radians(90.0f);
			break;
		case detail::TileRotation::By180:
			sprite.set_local().rotation = glm::radians(180.0f);
			break;
		case detail::TileRotation::By270:
			sprite.set_local().rotation = glm::radians(270.0f);
			break;
		default:
			sprite.set_local().rotation = 0.0f;
			break;
		}
		
		sprite.set_local().scale = 1.0f / context::get_texture_dimensions(tile_assignment.desc.file);
	}

	void TileMap::draw() const
	{
		for (const TileMapLayer& layer : layers)
			layer.draw();
	}

	void TileMap::set_camera_invariant(bool is_camera_invariant)
	{
		camera_invariant = is_camera_invariant;
		for (const TileMapLayer& layer : layers)
			layer.set_camera_invariant(is_camera_invariant);
	}

	bool TileMap::is_camera_invariant() const
	{
		return camera_invariant;
	}

	void TileMap::register_layer(TileMapLayer&& layer)
	{
		layer.transformer.attach_parent(&transformer);
		layer.set_camera_invariant(camera_invariant);
		layers.push_back(std::move(layer));
	}
		
	void TileMap::register_layer(size_t z, TileMapLayer&& layer)
	{
		layer.transformer.attach_parent(&transformer);
		layer.set_camera_invariant(camera_invariant);
		layers.insert(layers.begin() + z, std::move(layer));
	}

	TileMap TileMap::load(TOMLNode node)
	{
		assets::Parser parser(node);

		TileMap tilemap;
		if (auto transformer = parser.optional<TOMLNode>(detail::Key::Transformer)())
		{
			tilemap.set_local() = Transform2D::load(*transformer);
			tilemap.set_transformer().set_modifier() = TransformModifier2D::load(*transformer);
		}

		if (auto toml_layers = parser.optional<TOMLArray>(detail::Key::LayerArray)())
		{
			size_t _layer_idx = 0;
			toml_layers->for_each([&tilemap, &_layer_idx](auto&& node) {
				try
				{
					const size_t layer_idx = _layer_idx++;
					assets::Parser parser((TOMLNode)node, { "in tilemap layer #", layer_idx });

					auto tileset = parser.required<std::string>(detail::Key::TileSet)();

					TileMapLayer layer;
					layer.tileset = context::load_tileset(tileset);

					if (auto tiles = parser.optional<TOMLArray>(detail::Key::TileArray)())
					{
						size_t tile_idx = 0;
						for (auto& toml_tile : *tiles)
						{
							if (auto tile = assets::Parser((TOMLNode)toml_tile, { "in tile #", tile_idx, " from tilemap layer #", layer_idx }).optional<glm::ivec2>(assets::NO_KEY)())
								layer.paint_tile(*tile);
							++tile_idx;
						}
					}

					if (auto z = parser.optional<int>(detail::Key::Z)())
						tilemap.register_layer(*z, std::move(layer));
					else
						tilemap.register_layer(std::move(layer));
				}
				catch (const Error& e)
				{
					if (e.code != ErrorCode::LoadAsset)
						throw;
				}
				});
		}

		tilemap.set_camera_invariant(parser.defaulted(detail::Key::CameraInvariant)(false));

		return tilemap;
	}

	TileMap TileMap::load(TOMLNode node, const DebugTrace& trace)
	{
		auto scope = trace.scope("ASSETS", "oly::rendering::TileMap::load()");
		return load(node);
	}
}
