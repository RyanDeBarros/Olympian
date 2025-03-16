#include "Sprites.h"

#include "Resources.h"

#include <glm/gtc/type_ptr.hpp>
#include <limits>

oly::apollo::SpriteList::SpriteList(size_t quads_capacity, size_t textures_capacity, size_t uvs_capacity, const glm::vec4& projection_bounds)
{
	assert(4 * quads_capacity <= USHRT_MAX);
	assert(uvs_capacity <= 500);

	shader = shaders::sprite_list; // TODO lazy load shader on demand, rather than load(). It will check if shader has already been loaded or not.

	textures.resize(textures_capacity + 1); // extra 0th texture
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, tex_data_ssbo);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, textures.size() * sizeof(TexData), nullptr, GL_DYNAMIC_STORAGE_BIT);

	quads.resize(quads_capacity);

	quad_textures.resize(quads_capacity);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, quad_texture_ssbo);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, quad_textures.size() * sizeof(QuadTexInfo), quad_textures.data(), GL_DYNAMIC_STORAGE_BIT);

	quad_transforms.resize(quads_capacity, 1.0f);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, quad_transform_ssbo);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, quad_transforms.size() * sizeof(glm::mat3), quad_transforms.data(), GL_DYNAMIC_STORAGE_BIT);

	glBindBuffer(GL_UNIFORM_BUFFER, tex_coords_ubo);
	glBufferStorage(GL_UNIFORM_BUFFER, uvs_capacity * sizeof(TexUVRect), nullptr, GL_DYNAMIC_STORAGE_BIT);

	indices.resize(quads_capacity);
	set_draw_spec(0, quads_capacity);
	
	// TODO put in some kind of register/unregister quad functions
	for (GLushort i = 0; i < quads_capacity; ++i)
		rendering::quad_indices(indices[i].data, i);

	glBindVertexArray(vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(QuadIndexLayout), indices.data(), GL_DYNAMIC_STORAGE_BIT);

	set_projection(projection_bounds);
}

void oly::apollo::SpriteList::draw() const
{
	glUseProgram(*shader);
	glBindVertexArray(vao);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, tex_data_ssbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, quad_texture_ssbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, quad_transform_ssbo);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, tex_coords_ubo);
	glDrawElements(GL_TRIANGLES, draw_spec.count, GL_UNSIGNED_SHORT, (void*)(draw_spec.offset));
}

void oly::apollo::SpriteList::set_texture(const oly::rendering::TextureRes& texture, oly::rendering::ImageDimensions dim, size_t pos)
{
	assert(pos > 0); // cannot set 0th texture
	textures[pos] = texture;
	TexData texture_data;
	texture_data.dimensions = { dim.w, dim.h };
	texture_data.handle.use(*texture);
	glNamedBufferSubData(tex_data_ssbo, pos * sizeof(TexData), sizeof(TexData), &texture_data);
}

void oly::apollo::SpriteList::set_uvs(glm::vec2 bl, glm::vec2 br, glm::vec2 tr, glm::vec2 tl, size_t pos) const
{
	TexUVRect texture_coords;
	texture_coords.uvs[0] = bl;
	texture_coords.uvs[1] = br;
	texture_coords.uvs[2] = tr;
	texture_coords.uvs[3] = tl;
	glNamedBufferSubData(tex_coords_ubo, pos * sizeof(TexUVRect), sizeof(TexUVRect), &texture_coords);
}

void oly::apollo::SpriteList::set_projection(const glm::vec4& projection_bounds) const
{
	glm::mat3 proj = glm::ortho<float>(projection_bounds[0], projection_bounds[1], projection_bounds[2], projection_bounds[3]);
	GLuint proj_location = glGetUniformLocation(*shader, "uProjection");
	glUseProgram(*shader);
	glUniformMatrix3fv(proj_location, 1, GL_FALSE, glm::value_ptr(proj));
}

void oly::apollo::SpriteList::set_draw_spec(QuadPos first, QuadPos count)
{
	if (first < indices.size())
		draw_spec.first = first;
	draw_spec.count = 6 * std::min(count, indices.size() - draw_spec.first);
	draw_spec.offset = draw_spec.first * sizeof(QuadIndexLayout);
}

void oly::apollo::SpriteList::Quad::send_tex_info() const
{
	glNamedBufferSubData(sprite_list->quad_texture_ssbo, pos * sizeof(QuadTexInfo), sizeof(QuadTexInfo), sprite_list->quad_textures.data() + pos);
}

void oly::apollo::SpriteList::Quad::send_transform() const
{
	glNamedBufferSubData(sprite_list->quad_transform_ssbo, pos * sizeof(glm::mat3), sizeof(glm::mat3), sprite_list->quad_transforms.data() + pos);
}

void oly::apollo::SpriteList::Quad::send_data() const
{
	glNamedBufferSubData(sprite_list->quad_texture_ssbo, pos * sizeof(QuadTexInfo), sizeof(QuadTexInfo), sprite_list->quad_textures.data() + pos);
	glNamedBufferSubData(sprite_list->quad_transform_ssbo, pos * sizeof(glm::mat3), sizeof(glm::mat3), sprite_list->quad_transforms.data() + pos);
}

oly::apollo::SpriteList::Quad& oly::apollo::SpriteList::get_quad(QuadPos pos)
{
	Quad& quad = quads[pos];
	quad._tex_info = &quad_textures[pos];
	quad._transform = &quad_transforms[pos];
	quad.sprite_list = this;
	quad.pos = pos;
	return quad;
}

void oly::apollo::SpriteList::swap_quad_order(QuadPos pos1, QuadPos pos2)
{
	if (pos1 != pos2)
	{
		std::swap(indices[pos1], indices[pos2]);
		glNamedBufferSubData(ebo, pos1 * sizeof(QuadIndexLayout), sizeof(QuadIndexLayout), indices[pos1].data);
		glNamedBufferSubData(ebo, pos2 * sizeof(QuadIndexLayout), sizeof(QuadIndexLayout), indices[pos2].data);
		// TODO dirty flagging system for all buffer updates
	}
}

void oly::apollo::SpriteList::move_quad_order(QuadPos from, QuadPos to)
{
	if (from < to)
	{
		for (QuadPos i = from; i < to; ++i)
			swap_quad_order(i, i + 1);
	}
	else if (from > to)
	{
		for (QuadPos i = from; i > to; --i)
			swap_quad_order(i, i - 1);
	}
}
