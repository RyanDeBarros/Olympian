#pragma once

#include "SpecializedBuffers.h"
#include "math/Transforms.h"
#include "math/DataStructures.h"

#include <set>
#include <unordered_set>
#include <algorithm>

namespace oly
{
	namespace renderable
	{
		class Sprite;
	}

	namespace batch
	{
		class SpriteBatch
		{
			friend class renderable::Sprite;

			GLuint shader;
			rendering::VertexArray vao;
			rendering::QuadLayoutEBO ebo;

			GLuint projection_location;

			FixedVector<rendering::BindlessTextureRes> textures;

			struct TexData
			{
				GLuint64 handle = 0;
				glm::vec2 dimensions = {};
			};
			struct QuadInfo
			{
				GLuint tex_slot = 0;
				GLuint tex_coord_slot = 0;
				GLuint color_slot = 0;
			};
			rendering::LightweightSSBO<TexData, GLushort> tex_data_ssbo;
			rendering::IndexedSSBO<QuadInfo, GLushort> quad_info_ssbo;
			rendering::IndexedSSBO<glm::mat3, GLushort> quad_transform_ssbo;

		public:
			struct TexUVRect
			{
				glm::vec2 uvs[4] = {};
			};
			struct Modulation
			{
				glm::vec4 colors[4] = {};
			};
		private:
			rendering::LightweightUBO<TexUVRect, GLushort> tex_coords_ubo;
			rendering::LightweightUBO<Modulation, GLushort> modulation_ubo;

		public:
			struct Capacity
			{
				GLushort quads = 0;
				GLushort textures = 1;
				GLushort uvs = 1;
				GLushort modulations = 1;

				Capacity(GLushort quads, GLushort textures = 1, GLushort uvs = 1, GLushort modulations = 1);
			};

		private:
			const Capacity capacity;

		public:
			SpriteBatch(Capacity capacity, const glm::vec4& projection_bounds);

			void draw() const;

			void set_texture(GLushort pos, const rendering::BindlessTextureRes& texture, rendering::ImageDimensions dim);
			void refresh_handle(GLushort pos, rendering::ImageDimensions dim);
			void refresh_handle(GLushort pos);
			void set_uvs(GLushort pos, const TexUVRect& tex_coords) const;
			void set_modulation(GLushort pos, const Modulation& modulation) const;
			void set_projection(const glm::vec4& projection_bounds) const;

			typedef GLushort QuadPos;
		public:
			void get_draw_spec(QuadPos& first, QuadPos& count) const { ebo.get_draw_spec(first, count); }
			void set_draw_spec(QuadPos first, QuadPos count) { ebo.set_draw_spec(first, count); }

			class Quad
			{
				friend SpriteBatch;
				QuadInfo* _info = nullptr;
				glm::mat3* _transform = nullptr;
				SpriteBatch* _sprite_batch = nullptr;
				QuadPos _ssbo_pos = -1;

			public:
				Quad() = default;
				Quad(const Quad&) = delete;
				Quad(Quad&& other) noexcept = default;

				QuadInfo& info() { return *_info; }
				const QuadInfo& info() const { return *_info; }
				glm::mat3& transform() { return *_transform; }
				const glm::mat3& transform() const { return *_transform; }
				const SpriteBatch& sprite_batch() const { return *_sprite_batch; }
				SpriteBatch& sprite_batch() { return *_sprite_batch; }
				QuadPos index_pos() const { return _sprite_batch->z_order.range_of(_ssbo_pos); }
				void set_z_index(QuadPos z) { _sprite_batch->move_quad_order(index_pos(), z); }
				void move_z_index(int by) { _sprite_batch->move_quad_order(index_pos(), std::clamp((int)index_pos() + by, 0, (int)_sprite_batch->quads.size() - 1)); }

				void send_info() const;
				void send_transform() const;
				void send_data() const;
			};
			friend Quad;

		private:
			FixedVector<Quad> quads;
			math::IndexBijection<QuadPos> z_order;

		public:
			Quad& get_quad(QuadPos pos);
			void swap_quad_order(QuadPos pos1, QuadPos pos2);
			void move_quad_order(QuadPos from, QuadPos to);

			void flush() const;

		private:
			std::unordered_set<renderable::Sprite*> sprites;
		};
	}

	namespace renderable
	{
		class Sprite
		{
			friend batch::SpriteBatch;
			batch::SpriteBatch::Quad* _quad;
			std::unique_ptr<Transformer2D> _transformer;

		public:
			Sprite(batch::SpriteBatch* sprite_batch, batch::SpriteBatch::QuadPos pos);
			Sprite(batch::SpriteBatch* sprite_batch, batch::SpriteBatch::QuadPos pos, std::unique_ptr<Transformer2D>&& transformer);
			Sprite(const Sprite&) = delete;
			Sprite(Sprite&&) noexcept;
			~Sprite();
			Sprite& operator=(Sprite&&) noexcept;

			const batch::SpriteBatch& sprite_batch() const { return _quad->sprite_batch(); }
			batch::SpriteBatch& sprite_batch() { return _quad->sprite_batch(); }
			const batch::SpriteBatch::Quad& quad() const { return *_quad; }
			batch::SpriteBatch::Quad& quad() { return *_quad; }
			const Transformer2D& transformer() const { return *_transformer; }
			Transformer2D& transformer() { return *_transformer; }
			const Transform2D& local() const { return _transformer->local; }
			Transform2D& local() { return _transformer->local; }
			void post_set() const; // call after modifying local
			void pre_get() const; // call before reading global

		private:
			void flush() const;
		};
	}
}
