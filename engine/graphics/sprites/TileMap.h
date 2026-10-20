#pragma once

#include "core/base/TransformerExposure.h"

#include "graphics/sprites/Sprite.h"
#include "graphics/sprites/TileSet.h"

namespace oly::rendering
{
	struct TileMapLayer
        : public imp::ticket<internal::SpriteBatch>
	{
	private:
		using Super = imp::ticket<internal::SpriteBatch>;

        std::unordered_map<glm::ivec2, Sprite> sprite_map_;

	public:
		TileSetRef tileset;
		Transformer2D transformer;

	public:
		TileMapLayer();
		TileMapLayer(Unbatched);
		TileMapLayer(SpriteBatch& batch);
		TileMapLayer(const TileMapLayer&) = default;
		TileMapLayer(TileMapLayer&&) noexcept = default;

		TileMapLayer& operator=(const TileMapLayer&) = default;
		TileMapLayer& operator=(TileMapLayer&&) noexcept = default;

    public:
        void draw() const;

        const Transform2D& get_local() const;
        Transform2D& set_local();

		void set_camera_invariant(bool is_camera_invariant) const;

		auto get_batch() const { return lock(); }
		void set_batch(Unbatched);
		void set_batch(SpriteBatch& batch);

		void paint_tile(glm::ivec2 tile);
		void unpaint_tile(glm::ivec2 tile);

	private:
		void update_neighbour_configurations(glm::ivec2 center);
		void update_configuration(glm::ivec2 tile);
	};

	class TileMap
	{
		std::vector<TileMapLayer> layers_;
			
		Transformer2D transformer_;
		bool camera_invariant_ = false;

	public:
        static TileMap load(TOMLNode node);
        static TileMap load(TOMLNode node, const DebugTrace& trace);

        const TileMapLayer& layer(size_t i) const;
        TileMapLayer& layer(size_t i);
        const Transform2D& get_local() const;
        Transform2D& set_local();

        Transformer2DExposure<> get_transformer() const;
        Transformer2DExposure<ex::kAttachOnly> set_transformer();

		void draw() const;

		void set_camera_invariant(bool camera_invariant);
		bool is_camera_invariant() const;

		void register_layer(TileMapLayer&& layer);
		void register_layer(size_t z, TileMapLayer&& layer);
	};

	using TileMapRef = SmartReference<TileMap>;
}
