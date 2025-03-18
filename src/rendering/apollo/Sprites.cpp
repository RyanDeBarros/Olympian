#include "Sprites.h"

#include "Resources.h"

#include <glm/gtc/type_ptr.hpp>
#include <limits>

oly::apollo::SpriteList::SpriteList(Capacity capacity, const glm::vec4& projection_bounds)
	: capacity(capacity), z_order(4 * capacity.quads <= USHRT_MAX ? capacity.quads : 0)
{
	assert(4 * capacity.quads <= USHRT_MAX);
	assert(capacity.textures + 1 != 0); // there is enough capacity for 0th texture
	assert(1 <= capacity.uvs && capacity.uvs <= 500);
	assert(1 <= capacity.modulations && capacity.modulations <= 250);

	shader = shaders::sprite_list();

	++capacity.textures; // extra 0th texture
	textures.resize(capacity.textures + 1);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, tex_data_ssbo);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, textures.size() * sizeof(TexData), nullptr, GL_DYNAMIC_STORAGE_BIT);

	quads.resize(capacity.quads);

	quad_infos.resize(capacity.quads);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, quad_info_ssbo);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, quad_infos.size() * sizeof(QuadInfo), quad_infos.data(), GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);

	quad_transforms.resize(capacity.quads, 1.0f);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, quad_transform_ssbo);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, quad_transforms.size() * sizeof(glm::mat3), quad_transforms.data(), GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);

	glBindBuffer(GL_UNIFORM_BUFFER, tex_coords_ubo);
	glBufferStorage(GL_UNIFORM_BUFFER, capacity.uvs * sizeof(TexUVRect), nullptr, GL_DYNAMIC_STORAGE_BIT);
	TexUVRect tex_coords{ { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } } };
	glNamedBufferSubData(tex_coords_ubo, 0, sizeof(TexUVRect), &tex_coords);

	glBindBuffer(GL_UNIFORM_BUFFER, modulation_ubo);
	glBufferStorage(GL_UNIFORM_BUFFER, capacity.modulations * sizeof(Modulation), nullptr, GL_DYNAMIC_STORAGE_BIT);
	Modulation modulation{ { glm::vec4(1.0f), glm::vec4(1.0f), glm::vec4(1.0f), glm::vec4(1.0f) } };
	glNamedBufferSubData(modulation_ubo, 0, sizeof(Modulation), &modulation);

	indices.resize(capacity.quads);
	set_draw_spec(0, capacity.quads);
	
	for (GLushort i = 0; i < capacity.quads; ++i)
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
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, quad_info_ssbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, quad_transform_ssbo);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, tex_coords_ubo);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, modulation_ubo);
	glDrawElements(GL_TRIANGLES, (GLsizei)draw_spec.count, GL_UNSIGNED_SHORT, (void*)(draw_spec.offset));
}

void oly::apollo::SpriteList::set_texture(const oly::rendering::TextureRes& texture, oly::rendering::ImageDimensions dim, size_t pos)
{
	assert(pos > 0 && pos < capacity.textures); // cannot set 0th texture
	textures[pos] = texture;
	TexData texture_data;
	texture_data.dimensions = { dim.w, dim.h };
	texture_data.handle.use(*texture);
	glNamedBufferSubData(tex_data_ssbo, pos * sizeof(TexData), sizeof(TexData), &texture_data);
}

void oly::apollo::SpriteList::set_uvs(const TexUVRect& tex_coords, size_t pos) const
{
	assert(pos > 0 && pos < capacity.uvs); // cannot set 0th UV
	glNamedBufferSubData(tex_coords_ubo, pos * sizeof(TexUVRect), sizeof(TexUVRect), &tex_coords);
}

void oly::apollo::SpriteList::set_modulation(const Modulation& modulation, size_t pos) const
{
	assert(pos > 0 && pos < capacity.modulations); // cannot set 0th modulation
	glNamedBufferSubData(modulation_ubo, pos * sizeof(Modulation), sizeof(Modulation), &modulation);
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

void oly::apollo::SpriteList::Quad::send_info() const
{
	_sprite_list->dirty[Dirty::QUAD_INFO].insert(_ssbo_pos);
}

void oly::apollo::SpriteList::Quad::send_transform() const
{
	_sprite_list->dirty[Dirty::TRANSFORM].insert(_ssbo_pos);
}

void oly::apollo::SpriteList::Quad::send_data() const
{
	_sprite_list->dirty[Dirty::QUAD_INFO].insert(_ssbo_pos);
	_sprite_list->dirty[Dirty::TRANSFORM].insert(_ssbo_pos);
}

oly::apollo::SpriteList::Quad& oly::apollo::SpriteList::get_quad(QuadPos pos)
{
	Quad& quad = quads[pos];
	quad._info = &quad_infos[pos];
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
	for (Sprite* sprite : sprites)
		sprite->flush();
	process_set(Dirty::QUAD_INFO, quad_infos.data(), quad_info_ssbo, sizeof(QuadInfo));
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

oly::apollo::Sprite::Sprite(SpriteList* sprite_list, SpriteList::QuadPos pos)
	: sprite_list(sprite_list), _quad(&sprite_list->get_quad(pos)), _transformer(std::make_unique<Transformer2D>())
{
	sprite_list->sprites.insert(this);
}

oly::apollo::Sprite::Sprite(SpriteList* sprite_list, SpriteList::QuadPos pos, std::unique_ptr<Transformer2D>&& transformer)
	: sprite_list(sprite_list), _quad(&sprite_list->get_quad(pos)), _transformer(std::move(transformer))
{
	sprite_list->sprites.insert(this);
}

oly::apollo::Sprite::Sprite(Sprite&& other) noexcept
	: sprite_list(other.sprite_list), _quad(other._quad), _transformer(std::move(other._transformer))
{
}

oly::apollo::Sprite::~Sprite()
{
	if (sprite_list)
		sprite_list->sprites.erase(this);
}

oly::apollo::Sprite& oly::apollo::Sprite::operator=(Sprite&& other) noexcept
{
	if (this != &other)
	{
		sprite_list->sprites.erase(this);
		sprite_list = other.sprite_list;
		_quad = other._quad;
		_transformer = std::move(other._transformer);
		other.sprite_list = nullptr;
	}
	return *this;
}

void oly::apollo::Sprite::post_set() const
{
	_transformer->post_set();
}

void oly::apollo::Sprite::pre_get() const
{
	_transformer->pre_get();
}

void oly::apollo::Sprite::flush() const
{
	if (_transformer->flush())
	{
		_transformer->pre_get();
		_quad->transform() = _transformer->global();
		_quad->send_transform();
	}
}
