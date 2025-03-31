#include "Sprites.h"

#include "Resources.h"

#include <glm/gtc/type_ptr.hpp>
#include <limits>

namespace oly
{
	namespace batch
	{
		SpriteBatch::Capacity::Capacity(GLushort quads, GLushort textures, GLushort uvs, GLushort modulations)
			: quads(quads), textures(textures), uvs(uvs), modulations(modulations)
		{
			assert(4 * quads <= USHRT_MAX);
			assert(textures > 0); // there is enough capacity for 0th texture
			assert(0 < uvs && uvs <= 500);
			assert(0 < modulations && modulations <= 250);
		}

		SpriteBatch::SpriteBatch(Capacity capacity, const glm::vec4& projection_bounds)
			: ssbos(SSBO::__SSBO_COUNT), ubos(UBO::__UBO_COUNT), capacity(capacity), z_order(capacity.quads),
			quad_infos(capacity.quads), quad_transforms(capacity.quads, 1.0f), quads(capacity.quads), textures(capacity.textures), indices(capacity.quads)
		{
			shader = shaders::sprite_batch;
			glUseProgram(shader);
			projection_location = glGetUniformLocation(shader, "uProjection");

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbos[SSBO::B_TEX_DATA]);
			glNamedBufferStorage(ssbos[SSBO::B_TEX_DATA], capacity.textures * sizeof(TexData), nullptr, GL_DYNAMIC_STORAGE_BIT);

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbos[SSBO::B_QUAD_INFO]);
			glNamedBufferStorage(ssbos[SSBO::B_QUAD_INFO], capacity.quads * sizeof(QuadInfo), nullptr, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbos[SSBO::B_QUAD_TRANSFORM]);
			glNamedBufferStorage(ssbos[SSBO::B_QUAD_TRANSFORM], capacity.quads * sizeof(glm::mat3), nullptr, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);

			glBindBuffer(GL_UNIFORM_BUFFER, ubos[UBO::B_TEX_COORDS]);
			glNamedBufferStorage(ubos[UBO::B_TEX_COORDS], capacity.uvs * sizeof(TexUVRect), nullptr, GL_DYNAMIC_STORAGE_BIT);
			TexUVRect tex_coords{ { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } } };
			glNamedBufferSubData(ubos[UBO::B_TEX_COORDS], 0, sizeof(TexUVRect), &tex_coords);

			glBindBuffer(GL_UNIFORM_BUFFER, ubos[UBO::B_MODULATION]);
			glNamedBufferStorage(ubos[UBO::B_MODULATION], capacity.modulations * sizeof(Modulation), nullptr, GL_DYNAMIC_STORAGE_BIT);
			Modulation modulation{ { glm::vec4(1.0f), glm::vec4(1.0f), glm::vec4(1.0f), glm::vec4(1.0f) } };
			glNamedBufferSubData(ubos[UBO::B_MODULATION], 0, sizeof(Modulation), &modulation);

			set_draw_spec(0, capacity.quads);

			for (GLushort i = 0; i < capacity.quads; ++i)
				rendering::quad_indices(indices[i].data, i);

			glBindVertexArray(vao);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
			glNamedBufferStorage(ebo, capacity.quads * sizeof(QuadIndexLayout), indices.data(), GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);

			set_projection(projection_bounds);

			glBindVertexArray(0);
		}

		void SpriteBatch::draw() const
		{
			glUseProgram(shader);
			glBindVertexArray(vao);

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbos[SSBO::B_TEX_DATA]);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbos[SSBO::B_QUAD_INFO]);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbos[SSBO::B_QUAD_TRANSFORM]);
			glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubos[UBO::B_TEX_COORDS]);
			glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubos[UBO::B_MODULATION]);
			glDrawElements(GL_TRIANGLES, (GLsizei)draw_spec.count, GL_UNSIGNED_SHORT, (void*)(draw_spec.offset));
		}

		void SpriteBatch::set_texture(size_t pos, const rendering::BindlessTextureRes& texture, rendering::ImageDimensions dim)
		{
			assert(pos > 0 && pos < capacity.textures); // cannot set 0th texture
			textures[pos] = texture;
			texture->use_handle();
			TexData texture_data;
			texture_data.dimensions = { dim.w, dim.h };
			texture_data.handle = texture->get_handle();
			glNamedBufferSubData(ssbos[SSBO::B_TEX_DATA], pos * sizeof(TexData), sizeof(TexData), &texture_data); // TODO move to process()
		}

		void SpriteBatch::refresh_handle(size_t pos, rendering::ImageDimensions dim)
		{
			assert(pos > 0 && pos < capacity.textures); // cannot set 0th texture
			textures[pos]->use_handle();
			TexData texture_data;
			texture_data.dimensions = { dim.w, dim.h };
			texture_data.handle = textures[pos]->get_handle();
			glNamedBufferSubData(ssbos[SSBO::B_TEX_DATA], pos * sizeof(TexData), sizeof(TexData), &texture_data); // TODO move to process()
		}

		void SpriteBatch::refresh_handle(size_t pos)
		{
			assert(pos > 0 && pos < capacity.textures); // cannot set 0th texture
			textures[pos]->use_handle();
			GLuint64 handle = textures[pos]->get_handle();
			glNamedBufferSubData(ssbos[SSBO::B_TEX_DATA], pos * sizeof(TexData) + offsetof(TexData, handle), sizeof(GLuint64), &handle); // TODO move to process()
		}

		void SpriteBatch::set_uvs(size_t pos, const TexUVRect& tex_coords) const
		{
			assert(pos > 0 && pos < capacity.uvs); // cannot set 0th UV
			glNamedBufferSubData(ubos[UBO::B_TEX_COORDS], pos * sizeof(TexUVRect), sizeof(TexUVRect), &tex_coords);
		}

		void SpriteBatch::set_modulation(size_t pos, const Modulation& modulation) const
		{
			assert(pos > 0 && pos < capacity.modulations); // cannot set 0th modulation
			glNamedBufferSubData(ubos[UBO::B_MODULATION], pos * sizeof(Modulation), sizeof(Modulation), &modulation);
		}

		void SpriteBatch::set_projection(const glm::vec4& projection_bounds) const
		{
			glm::mat3 proj = glm::ortho<float>(projection_bounds[0], projection_bounds[1], projection_bounds[2], projection_bounds[3]);
			glUseProgram(shader);
			glUniformMatrix3fv(projection_location, 1, GL_FALSE, glm::value_ptr(proj));
		}

		void SpriteBatch::set_draw_spec(QuadPos first, QuadPos count)
		{
			if (first < indices.size())
				draw_spec.first = first;
			draw_spec.count = 6 * std::min(count, (QuadPos)(indices.size() - draw_spec.first));
			draw_spec.offset = draw_spec.first * sizeof(QuadIndexLayout);
		}

		void SpriteBatch::Quad::send_info() const
		{
			_sprite_batch->dirty_quad_infos.insert(_ssbo_pos);
		}

		void SpriteBatch::Quad::send_transform() const
		{
			_sprite_batch->dirty_transforms.insert(_ssbo_pos);
		}

		void SpriteBatch::Quad::send_data() const
		{
			_sprite_batch->dirty_quad_infos.insert(_ssbo_pos);
			_sprite_batch->dirty_transforms.insert(_ssbo_pos);
		}

		SpriteBatch::Quad& SpriteBatch::get_quad(QuadPos pos)
		{
			Quad& quad = quads[pos];
			quad._info = &quad_infos[pos];
			quad._transform = &quad_transforms[pos];
			quad._sprite_batch = this;
			quad._ssbo_pos = pos;
			return quad;
		}

		void SpriteBatch::swap_quad_order(QuadPos pos1, QuadPos pos2)
		{
			if (pos1 != pos2)
			{
				std::swap(indices[pos1], indices[pos2]);
				z_order.swap_range(pos1, pos2);
				dirty_indices.insert(pos1);
				dirty_indices.insert(pos2);
			}
		}

		void SpriteBatch::move_quad_order(QuadPos from, QuadPos to)
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

		void SpriteBatch::process()
		{
			for (renderable::Sprite* sprite : sprites)
				sprite->flush();
			process_set(dirty_quad_infos, Dirty::D_QUAD_INFO, quad_infos.data(), ssbos[SSBO::B_QUAD_INFO], sizeof(QuadInfo));
			process_set(dirty_transforms, Dirty::D_TRANSFORM, quad_transforms.data(), ssbos[SSBO::B_QUAD_TRANSFORM], sizeof(glm::mat3));
			process_set(dirty_indices, Dirty::D_INDICES, indices.data(), ebo, sizeof(QuadIndexLayout));
		}

		// TODO move to utility function
		void SpriteBatch::process_set(std::set<QuadPos>& set, Dirty flag, void* _data, GLuint buf, size_t element_size)
		{
			std::byte* data = (std::byte*)_data;
			switch (send_types[flag])
			{
			case BufferSendType::SUBDATA:
			{
				bool contiguous = false;
				GLintptr offset = 0;
				GLsizeiptr size = 0;
				for (auto iter = set.begin(); iter != set.end(); ++iter)
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
				for (auto iter = set.begin(); iter != set.end(); ++iter)
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
			set.clear();
		}
	}

	namespace renderable
	{
		Sprite::Sprite(batch::SpriteBatch* sprite_batch, batch::SpriteBatch::QuadPos pos)
			: _quad(&sprite_batch->get_quad(pos)), _transformer(std::make_unique<Transformer2D>())
		{
			sprite_batch->sprites.insert(this);
		}

		Sprite::Sprite(batch::SpriteBatch* sprite_batch, batch::SpriteBatch::QuadPos pos, std::unique_ptr<Transformer2D>&& transformer)
			: _quad(&sprite_batch->get_quad(pos)), _transformer(std::move(transformer))
		{
			sprite_batch->sprites.insert(this);
		}

		Sprite::Sprite(Sprite&& other) noexcept
			: _quad(other._quad), _transformer(std::move(other._transformer))
		{
			if (other._quad)
			{
				other._quad->sprite_batch().sprites.erase(&other);
				other._quad = nullptr;
			}
		}

		Sprite::~Sprite()
		{
			if (_quad)
				_quad->sprite_batch().sprites.erase(this);
		}

		Sprite& Sprite::operator=(Sprite&& other) noexcept
		{
			if (this != &other)
			{
				if (_quad)
					_quad->sprite_batch().sprites.erase(this);
				_quad = other._quad;
				if (other._quad)
				{
					other._quad->sprite_batch().sprites.erase(&other);
					other._quad = nullptr;
				}
				_transformer = std::move(other._transformer);
			}
			return *this;
		}

		void Sprite::post_set() const
		{
			_transformer->post_set();
		}

		void Sprite::pre_get() const
		{
			_transformer->pre_get();
		}

		void Sprite::flush() const
		{
			if (_transformer->flush())
			{
				_transformer->pre_get();
				_quad->transform() = _transformer->global();
				_quad->send_transform();
			}
		}
	}
}
