#include "Sprites.h"

#include "Resources.h"

#include <glm/gtc/type_ptr.hpp>
#include <limits>

oly::apollo::SpriteList::SpriteList(size_t quads_capacity, size_t textures_capacity, size_t uvs_capacity, const glm::vec4& projection_bounds)
	: z_order(4 * quads_capacity <= USHRT_MAX ? quads_capacity : 0)
{
	assert(4 * quads_capacity <= USHRT_MAX);
	assert(uvs_capacity <= 500);

	shader = shaders::sprite_list(); // TODO lazy load shader on demand, rather than load(). It will check if shader has already been loaded or not.

	textures.resize(textures_capacity + 1); // extra 0th texture
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, tex_data_ssbo);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, textures.size() * sizeof(TexData), nullptr, GL_DYNAMIC_STORAGE_BIT);

	quads.resize(quads_capacity);

	quad_textures.resize(quads_capacity);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, quad_texture_ssbo);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, quad_textures.size() * sizeof(QuadTexInfo), quad_textures.data(), GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);

	quad_transforms.resize(quads_capacity, 1.0f);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, quad_transform_ssbo);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, quad_transforms.size() * sizeof(glm::mat3), quad_transforms.data(), GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);

	glBindBuffer(GL_UNIFORM_BUFFER, tex_coords_ubo);
	glBufferStorage(GL_UNIFORM_BUFFER, uvs_capacity * sizeof(TexUVRect), nullptr, GL_DYNAMIC_STORAGE_BIT);

	indices.resize(quads_capacity);
	set_draw_spec(0, quads_capacity);
	
	for (GLushort i = 0; i < quads_capacity; ++i)
		rendering::quad_indices(indices[i].data, i);

	glBindVertexArray(vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(QuadIndexLayout), indices.data(), GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);

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
	draw_spec.count = 6 * std::min(count, (QuadPos)indices.size() - draw_spec.first);
	draw_spec.offset = draw_spec.first * sizeof(QuadIndexLayout);
}

void oly::apollo::SpriteList::Quad::send_tex_info() const
{
	_sprite_list->dirty[Dirty::TEX_INFO].insert(_ssbo_pos);
}

void oly::apollo::SpriteList::Quad::send_transform() const
{
	_sprite_list->dirty[Dirty::TRANSFORM].insert(_ssbo_pos);
}

void oly::apollo::SpriteList::Quad::send_data() const
{
	_sprite_list->dirty[Dirty::TEX_INFO].insert(_ssbo_pos);
	_sprite_list->dirty[Dirty::TRANSFORM].insert(_ssbo_pos);
}

oly::apollo::SpriteList::Quad& oly::apollo::SpriteList::get_quad(QuadPos pos)
{
	Quad& quad = quads[pos];
	quad._tex_info = &quad_textures[pos];
	quad._transform = &quad_transforms[pos];
	quad._sprite_list = this;
	quad._ssbo_pos = pos;
	return quad;
}

void oly::apollo::SpriteList::swap_quad_order(QuadPos pos1, QuadPos pos2)
{
	if (pos1 != pos2)
	{
		std::swap(indices[pos1], indices[pos2]);
		z_order.swap_range(pos1, pos2);
		dirty[Dirty::INDICES].insert(pos1);
		dirty[Dirty::INDICES].insert(pos2);
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

void oly::apollo::SpriteList::process()
{
	process_set(Dirty::TEX_INFO, quad_textures.data(), quad_texture_ssbo, sizeof(QuadTexInfo));
	process_set(Dirty::TRANSFORM, quad_transforms.data(), quad_transform_ssbo, sizeof(glm::mat3));
	process_set(Dirty::INDICES, indices.data(), ebo, sizeof(QuadIndexLayout));
}

void oly::apollo::SpriteList::process_set(Dirty flag, void* _data, GLuint buf, size_t element_size)
{
	std::byte* data = (std::byte*)_data;
	switch (send_types[flag])
	{
	case BufferSendType::SUBDATA:
	{
		bool contiguous = false;
		GLintptr offset = 0;
		GLsizeiptr size = 0;
		for (auto iter = dirty[flag].begin(); iter != dirty[flag].end(); ++iter)
		{
			if (contiguous)
			{
				if (*iter * element_size == offset + size)
					size += element_size;
				else
				{
					glNamedBufferSubData(buf, offset, size, data + offset);
					contiguous = false;
				}
			}
			else
			{
				offset = *iter * element_size;
				size = element_size;
				contiguous = true;
			}
		}
		if (contiguous)
			glNamedBufferSubData(buf, offset, size, data + offset);
		break;
	}
	case BufferSendType::MAP:
	{
		std::byte* gpu_buf = (std::byte*)glMapNamedBuffer(buf, GL_WRITE_ONLY);
		bool contiguous = false;
		GLintptr offset = 0;
		GLsizeiptr size = 0;
		for (auto iter = dirty[flag].begin(); iter != dirty[flag].end(); ++iter)
		{
			if (contiguous)
			{
				if (*iter * element_size == offset + size)
					size += element_size;
				else
				{
					memcpy(gpu_buf + offset, data + offset, size);
					contiguous = false;
				}
			}
			else
			{
				offset = *iter * element_size;
				size = element_size;
				contiguous = true;
			}
		}
		if (contiguous)
			memcpy(gpu_buf + offset, data + offset, size);
		glUnmapNamedBuffer(buf);
		break;
	}
	}
	dirty[flag].clear();
}
