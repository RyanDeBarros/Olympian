#pragma once

#include "../core/Core.h"
#include "util/MathDS.h"

#include <set>

namespace oly
{
	namespace apollo
	{
		class SpriteList
		{
			oly::rendering::ShaderRes shader;
			oly::rendering::VertexArray vao;
			oly::rendering::GLBuffer ebo;

			std::vector<oly::rendering::TextureRes> textures;

			struct TexData
			{
				oly::rendering::BindlessTextureHandle handle;
				glm::vec2 dimensions = {};
			};
			struct QuadTexInfo
			{
				GLuint tex_slot = 0;
				GLuint tex_coord_slot = 0;
			};
			std::vector<QuadTexInfo> quad_textures;
			std::vector<glm::mat3> quad_transforms;

			oly::rendering::GLBuffer tex_data_ssbo;
			oly::rendering::GLBuffer quad_texture_ssbo;
			oly::rendering::GLBuffer quad_transform_ssbo;

			struct TexUVRect
			{
				glm::vec2 uvs[4] = {};
			};
			oly::rendering::GLBuffer tex_coords_ubo;

			struct QuadIndexLayout
			{
				GLushort data[6];
			};
			std::vector<QuadIndexLayout> indices;

		public:
			SpriteList(size_t quads_capacity, size_t textures_capacity, size_t uvs_capacity, const glm::vec4& projection_bounds);

			void draw() const;

			GLuint get_shader() const { return *shader; }
			void set_texture(const oly::rendering::TextureRes& texture, oly::rendering::ImageDimensions dim, size_t pos);
			void set_uvs(glm::vec2 bl, glm::vec2 br, glm::vec2 tr, glm::vec2 tl, size_t pos) const;
			void set_projection(const glm::vec4& projection_bounds) const;

			typedef size_t QuadPos;
		private:
			struct
			{
				QuadPos first = 0;
				QuadPos count = 0;
				size_t offset = 0;
			} draw_spec;

		public:
			void get_draw_spec(QuadPos& first, QuadPos& count) const { first = draw_spec.first; count = draw_spec.count; }
			void set_draw_spec(QuadPos first, QuadPos count);

			class Quad
			{
				friend SpriteList;
				QuadTexInfo* _tex_info = nullptr;
				glm::mat3* _transform = nullptr;
				SpriteList* _sprite_list = nullptr;
				QuadPos _ssbo_pos = -1;

			public:
				Quad() = default;
				Quad(const Quad&) = delete;
				Quad(Quad&& other) noexcept = default;

				QuadTexInfo& tex_info() { return *_tex_info; }
				const QuadTexInfo& tex_info() const { return *_tex_info; }
				glm::mat3& transform() { return *_transform; }
				const glm::mat3& transform() const { return *_transform; }
				const SpriteList& sprite_list() const { return *_sprite_list; }
				SpriteList& sprite_list() { return *_sprite_list; }
				QuadPos index_pos() const { return _sprite_list->z_order.range_of(_ssbo_pos); }

				void send_tex_info() const;
				void send_transform() const;
				void send_data() const;
			};
			friend Quad;

		private:
			std::vector<Quad> quads;
			math::IndexBijection<QuadPos> z_order;

		public:
			Quad& get_quad(QuadPos pos);
			void swap_quad_order(QuadPos pos1, QuadPos pos2);
			void move_quad_order(QuadPos from, QuadPos to);

			enum Dirty
			{
				TEX_INFO,
				TRANSFORM,
				INDICES
			};

		private:
			std::set<QuadPos> dirty[3] = { {}, {}, {} };

		public:
			enum class BufferSendType
			{
				SUBDATA,
				MAP
			};
			BufferSendType send_types[3] = { BufferSendType::SUBDATA, BufferSendType::SUBDATA, BufferSendType::SUBDATA };

			void process();

		private:
			void process_set(Dirty flag, void* data, GLuint buf, size_t element_size);
		};
	}
}
