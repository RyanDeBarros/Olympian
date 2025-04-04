#pragma once

#include "SpecializedBuffers.h"
#include "math/Transforms.h"
#include "math/DataStructures.h"
#include "util/IDGenerator.h"

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

			FixedVector<rendering::BindlessTextureRes> textures;

			GLuint projection_location;
			GLuint modulation_location;

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

			void draw(size_t draw_spec = 0);

			void set_texture(GLushort pos, const rendering::BindlessTextureRes& texture, rendering::ImageDimensions dim);
			void refresh_handle(GLushort pos, rendering::ImageDimensions dim);
			void refresh_handle(GLushort pos);
			void set_uvs(GLushort pos, const TexUVRect& tex_coords) const;
			void set_modulation(GLushort pos, const Modulation& modulation) const;
			void set_projection(const glm::vec4& projection_bounds) const;
			void set_global_modulation(const glm::vec4& modulation) const;

			typedef GLushort QuadPos;
			std::vector<Range<QuadPos>> draw_specs;

			class QuadReference
			{
				friend SpriteBatch;
				SpriteBatch* _batch = nullptr;
				QuadPos _ssbo_pos = -1;
				bool active = true;
				QuadInfo* _info = nullptr;
				glm::mat3* _transform = nullptr;

			public:
				float z_value = 0.0f;

				QuadReference(SpriteBatch* batch);
				QuadReference(const QuadReference&) = delete;
				QuadReference(QuadReference&&) noexcept;
				~QuadReference();
				QuadReference& operator=(QuadReference&&) noexcept;

				const SpriteBatch& batch() const { return *_batch; }
				SpriteBatch& batch() { return *_batch; }
				QuadInfo& info() { return *_info; }
				const QuadInfo& info() const { return *_info; }
				glm::mat3& transform() { return *_transform; }
				const glm::mat3& transform() const { return *_transform; }
				
			private:
				QuadPos index_pos() const { return _batch->z_order.range_of(_ssbo_pos); }
				void set_z_index(QuadPos z) { _batch->move_quad_order(index_pos(), z); }
				void move_z_index(int by) { _batch->move_quad_order(index_pos(), index_pos() + by); }

			public:
				void send_info() const;
				void send_transform() const;
				void send_data() const;
			};
			friend QuadReference;

		private:
			math::IndexBijection<QuadPos> z_order;
			IDGenerator<QuadPos> pos_generator;

		public:
			void swap_quad_order(QuadPos pos1, QuadPos pos2);
			void move_quad_order(QuadPos from, QuadPos to);
			void sync_z_values() { dirty_z = true; }
			
		private:
			bool dirty_z = false;

		public:
			void flush();

		private:
			std::set<renderable::Sprite*> sprites;
			void flush_z_values();
		};
	}

	namespace renderable
	{
		class Sprite
		{
			friend batch::SpriteBatch;
			std::unique_ptr<Transformer2D> _transformer;

		public:
			batch::SpriteBatch::QuadReference quad;

			Sprite(batch::SpriteBatch* sprite_batch);
			Sprite(batch::SpriteBatch* sprite_batch, std::unique_ptr<Transformer2D>&& transformer);
			Sprite(const Sprite&) = delete;
			Sprite(Sprite&&) noexcept;
			~Sprite();
			Sprite& operator=(Sprite&&) noexcept;

			const batch::SpriteBatch& batch() const { return quad.batch(); }
			batch::SpriteBatch& batch() { return quad.batch(); }
			const Transformer2D& transformer() const { return *_transformer; }
			Transformer2D& transformer() { return *_transformer; }
			const Transform2D& local() const { return _transformer->local; }
			Transform2D& local() { return _transformer->local; }
			void post_set() const; // call after modifying local
			void pre_get() const; // call before reading global

		private:
			void flush();
		};
	}
}
