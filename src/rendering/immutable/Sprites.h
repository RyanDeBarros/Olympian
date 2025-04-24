#pragma once

#include "../SpecializedBuffers.h"
#include "util/IDGenerator.h"
#include "math/Transforms.h"
#include "math/DataStructures.h"

#include <set>
#include <unordered_set>
#include <algorithm>

namespace oly
{
	namespace immut
	{
		struct Sprite;

		class SpriteBatch
		{
			friend struct Sprite;

			rendering::VertexArray vao;
			rendering::QuadLayoutEBO<rendering::Mutability::IMMUTABLE> ebo;

			FixedVector<rendering::BindlessTextureRes> textures;

			struct SSBO
			{
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
					GLuint frame_slot = 0;
				};

				rendering::LightweightSSBO<rendering::Mutability::IMMUTABLE> tex_data;
				rendering::IndexedSSBO<QuadInfo, GLushort, rendering::Mutability::IMMUTABLE> quad_info;
				rendering::IndexedSSBO<glm::mat3, GLushort, rendering::Mutability::IMMUTABLE> quad_transform;

				SSBO(GLushort textures, GLushort quads) : tex_data(textures * sizeof(TexData)), quad_info(quads), quad_transform(quads, 1.0f) {}
			} ssbo;

			struct
			{
				GLuint projection, modulation, time;
			} shader_locations;
		public:
			struct TexUVRect
			{
				glm::vec2 uvs[4] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
			};
			struct Modulation
			{
				glm::vec4 colors[4] = { glm::vec4(1.0f), glm::vec4(1.0f), glm::vec4(1.0f), glm::vec4(1.0f) };
			};
		private:
			struct UBO
			{
				rendering::LightweightUBO<rendering::Mutability::IMMUTABLE> tex_coords, modulation, anim;
				
				UBO(GLushort uvs, GLushort modulations, GLushort anims) : tex_coords(uvs * sizeof(TexUVRect)), modulation(modulations * sizeof(Modulation)), anim(anims * sizeof(rendering::AnimFrameFormat)) {}
			} ubo;

		public:
			struct Capacity
			{
				Capacity(GLushort quads, GLushort textures = 0, GLushort uvs = 0, GLushort modulations = 0, GLushort anims = 0);

			private:
				friend class SpriteBatch;
				GLushort quads, textures, uvs, modulations, anims;
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
			void set_frame_format(GLushort pos, const rendering::AnimFrameFormat& anim) const;

			glm::vec4 projection_bounds;
			glm::vec4 global_modulation = glm::vec4(1.0f);

			typedef GLushort QuadPos;
			typedef StrictIDGenerator<GLushort>::ID QID;
			std::vector<Range<QuadPos>> draw_specs;

			class QuadReference
			{
				friend SpriteBatch;
				SpriteBatch* _batch = nullptr;
				QID pos;
				bool active = true;
			
			public:
				float z_value = 0.0f;

			private:
				SSBO::QuadInfo* _info = nullptr;
				glm::mat3* _transform = nullptr;

			public:

				QuadReference(SpriteBatch* batch);
				QuadReference(const QuadReference&) = delete;
				QuadReference(QuadReference&&) noexcept;
				~QuadReference();
				QuadReference& operator=(QuadReference&&) noexcept;

				const SpriteBatch& batch() const { return *_batch; }
				SpriteBatch& batch() { return *_batch; }
				const SSBO::QuadInfo& info() const { return *_info; }
				SSBO::QuadInfo& info() { return *_info; }
				const glm::mat3& transform() const { return *_transform; }
				glm::mat3& transform() { return *_transform; }
				
			private:
				QuadPos index_pos() const { return _batch->z_order.range_of(pos.get()); }
				void set_z_index(QuadPos z) { _batch->move_quad_order(index_pos(), z); }
				void move_z_index(int by) { _batch->move_quad_order(index_pos(), index_pos() + by); }

			public:
				void send_info() const;
				void send_transform() const;
				void send_data() const;
				void send_z_value() { _batch->dirty_z = true; }
			};
			friend QuadReference;

		private:
			math::IndexBijection<QuadPos> z_order;
			StrictIDGenerator<QuadPos> pos_generator;

		public:
			void swap_quad_order(QuadPos pos1, QuadPos pos2);
			void move_quad_order(QuadPos from, QuadPos to);

			void flush();

		private:
			bool dirty_z = false;
			std::vector<QuadReference*> quad_refs;
			std::unordered_set<Sprite*> sprites;
			void flush_z_values();
		};

		struct Sprite
		{
			SpriteBatch::QuadReference quad;
			Transformer2D transformer;

			Sprite(SpriteBatch* sprite_batch);
			Sprite(const Sprite&) = delete;
			Sprite(Sprite&&) noexcept;
			~Sprite();
			Sprite& operator=(Sprite&&) noexcept;

			const SpriteBatch& batch() const { return quad.batch(); }
			SpriteBatch& batch() { return quad.batch(); }
			const Transform2D& local() const { return transformer.local; }
			Transform2D& local() { return transformer.local; }
			void post_set(); // call after modifying local
			void pre_get() const; // call before reading global

		private:
			friend SpriteBatch;
			void flush();
		};
	}
}
