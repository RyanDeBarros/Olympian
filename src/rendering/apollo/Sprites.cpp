#include "Sprites.h"

oly::apollo::SpriteListBatch::SpriteListBatch(size_t num_quads, size_t num_textures, size_t num_uvs)
{
	assert(num_uvs <= 500);

	textures.resize(num_textures + 1); // extra 0th texture
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, tex_data_ssbo);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, textures.size() * sizeof(TexData), nullptr, GL_DYNAMIC_STORAGE_BIT);

	quad_textures.resize(num_quads);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, quad_texture_ssbo);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, quad_textures.size() * sizeof(QuadTexInfo), quad_textures.data(), GL_DYNAMIC_STORAGE_BIT);

	quad_transforms.resize(num_quads, glm::mat3(1.0f));
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, quad_transform_ssbo);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, quad_transforms.size() * sizeof(glm::mat3), quad_transforms.data(), GL_DYNAMIC_STORAGE_BIT);

	glBindBuffer(GL_UNIFORM_BUFFER, tex_coords_ubo);
	glBufferStorage(GL_UNIFORM_BUFFER, num_uvs * sizeof(TexUVRect), nullptr, GL_DYNAMIC_STORAGE_BIT);

	indices.resize(6 * num_quads);
	// TODO note that this draws all quads and relies on primitive discarding based on tex slot == 0 for inivisible (unregistered) quads. This system is simpler than a dynamic sprite list, but is less efficient.
	draw_spec.indices = 6 * num_quads;
	
	// TODO put in some kind of register/unregister quad functions
	rendering::quad_indices(indices.data(), (GLushort)0);
	rendering::quad_indices(indices.data() + 6, (GLushort)1);

	glBindVertexArray(vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLushort), indices.data(), GL_DYNAMIC_STORAGE_BIT);
}

void oly::apollo::SpriteListBatch::draw() const
{
	glUseProgram(*shader);
	glBindVertexArray(vao);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, tex_data_ssbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, quad_texture_ssbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, quad_transform_ssbo);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, tex_coords_ubo);
	glDrawElements(draw_spec.mode, draw_spec.indices, draw_spec.type, (void*)(draw_spec.offset * sizeof(GLushort)));
}

void oly::apollo::SpriteListBatch::set_texture(const oly::rendering::TextureRes& texture, oly::rendering::ImageDimensions dim, size_t pos)
{
	assert(pos > 0); // cannot set 0th texture
	textures[pos] = texture;
	TexData texture_data;
	texture_data.dimensions = { dim.w, dim.h };
	texture_data.handle.use(*texture);
	glNamedBufferSubData(tex_data_ssbo, pos * sizeof(TexData), sizeof(TexData), &texture_data);
}

void oly::apollo::SpriteListBatch::set_uvs(glm::vec2 bl, glm::vec2 br, glm::vec2 tr, glm::vec2 tl, size_t pos) const
{
	TexUVRect texture_coords;
	texture_coords.uvs[0] = bl;
	texture_coords.uvs[1] = br;
	texture_coords.uvs[2] = tr;
	texture_coords.uvs[3] = tl;
	glNamedBufferSubData(tex_coords_ubo, pos * sizeof(TexUVRect), sizeof(TexUVRect), &texture_coords);
}

oly::apollo::SpriteListBatch::Quad oly::apollo::SpriteListBatch::get_quad(size_t pos)
{
	Quad quad;
	quad.tex_info = &quad_textures[pos];
	quad.transform = &quad_transforms[pos];
	return quad;
}

void oly::apollo::SpriteListBatch::send_quad_data(size_t pos)
{
	glNamedBufferSubData(quad_texture_ssbo, pos * sizeof(TexData), sizeof(TexData), quad_textures.data() + pos);
	glNamedBufferSubData(quad_transform_ssbo, pos * sizeof(glm::mat3), sizeof(glm::mat3), quad_transforms.data() + pos);
}
