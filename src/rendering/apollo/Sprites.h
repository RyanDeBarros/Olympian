#pragma once

#include "../core/Core.h"

namespace oly
{
	namespace apollo
	{
		struct SpriteListBatch
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

			struct Quad
			{
				QuadTexInfo* tex_info = nullptr;
				glm::mat3* transform = nullptr;
			};

			std::vector<GLushort> indices;

			struct
			{
				GLenum mode = GL_TRIANGLES;
				GLuint indices = 12;
				GLenum type = GL_UNSIGNED_SHORT;
				GLuint offset = 0;
			} draw_spec;

			SpriteListBatch(size_t num_quads, size_t num_textures, size_t num_uvs);

			void draw() const;

			void set_texture(const oly::rendering::TextureRes& texture, oly::rendering::ImageDimensions dim, size_t pos);
			void set_uvs(glm::vec2 bl, glm::vec2 br, glm::vec2 tr, glm::vec2 tl, size_t pos) const;
			Quad get_quad(size_t pos);
			void send_quad_data(size_t pos);
		};
	}
}
