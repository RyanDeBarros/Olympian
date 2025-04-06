#include "Sprites.h"

#include <glm/gtc/type_ptr.hpp>

#include <limits>

#include "Resources.h"

namespace oly
{
	namespace batch
	{
		SpriteBatch::Capacity::Capacity(GLushort quads, GLushort textures, GLushort uvs, GLushort modulations)
			: quads(quads), textures(textures), uvs(uvs), modulations(modulations)
		{
			OLY_ASSERT(4 * quads <= USHRT_MAX);
			OLY_ASSERT(textures > 0); // there is enough capacity for 0th texture
			OLY_ASSERT(0 < uvs && uvs <= 500);
			OLY_ASSERT(0 < modulations && modulations <= 250);
		}

		SpriteBatch::SpriteBatch(Capacity capacity, const glm::vec4& projection_bounds)
			: ebo(capacity.quads), capacity(capacity), tex_data_ssbo(capacity.textures), quad_info_ssbo(capacity.quads), quad_transform_ssbo(capacity.quads, 1.0f), tex_coords_ubo(capacity.uvs), modulation_ubo(capacity.modulations),
			z_order(capacity.quads), textures(capacity.textures)
		{
			shader = shaders::sprite_batch;
			glUseProgram(shader);
			projection_location = glGetUniformLocation(shader, "uProjection");
			modulation_location = glGetUniformLocation(shader, "uGlobalModulation");

			tex_coords_ubo.send(0, { { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } } });
			modulation_ubo.send(0, { { glm::vec4(1.0f), glm::vec4(1.0f), glm::vec4(1.0f), glm::vec4(1.0f) } });

			glBindVertexArray(vao);
			rendering::pre_init(ebo);
			ebo.init();
			glBindVertexArray(0);

			set_projection(projection_bounds);
			set_global_modulation(glm::vec4(1.0f));
			draw_specs.push_back({ 0, capacity.quads });
		}

		void SpriteBatch::draw(size_t draw_spec)
		{
			glUseProgram(shader);
			glBindVertexArray(vao);

			tex_data_ssbo.bind_base(0);
			quad_info_ssbo.bind_base(1);
			quad_transform_ssbo.bind_base(2);
			tex_coords_ubo.bind_base(0);
			modulation_ubo.bind_base(1);
			ebo.set_draw_spec(draw_specs[draw_spec].initial, draw_specs[draw_spec].length);
			ebo.draw(GL_TRIANGLES, GL_UNSIGNED_SHORT);
		}

		void SpriteBatch::set_texture(GLushort pos, const rendering::BindlessTextureRes& texture, rendering::ImageDimensions dim)
		{
			OLY_ASSERT(pos > 0 && pos < capacity.textures); // cannot set 0th texture
			textures[pos] = texture;
			texture->use_handle();
			TexData texture_data;
			texture_data.dimensions = { dim.w, dim.h };
			texture_data.handle = texture->get_handle();
			tex_data_ssbo.send(pos, texture_data);
		}

		void SpriteBatch::refresh_handle(GLushort pos, rendering::ImageDimensions dim)
		{
			OLY_ASSERT(pos > 0 && pos < capacity.textures); // cannot set 0th texture
			textures[pos]->use_handle();
			TexData texture_data;
			texture_data.dimensions = { dim.w, dim.h };
			texture_data.handle = textures[pos]->get_handle();
			tex_data_ssbo.send(pos, texture_data);
		}

		void SpriteBatch::refresh_handle(GLushort pos)
		{
			OLY_ASSERT(pos > 0 && pos < capacity.textures); // cannot set 0th texture
			textures[pos]->use_handle();
			GLuint64 handle = textures[pos]->get_handle();
			tex_data_ssbo.send(pos, &TexData::handle, handle);
		}

		void SpriteBatch::set_uvs(GLushort pos, const TexUVRect& tex_coords) const
		{
			OLY_ASSERT(pos > 0 && pos < capacity.uvs); // cannot set 0th UV
			tex_coords_ubo.send(pos, tex_coords);
		}

		void SpriteBatch::set_modulation(GLushort pos, const Modulation& modulation) const
		{
			OLY_ASSERT(pos > 0 && pos < capacity.modulations); // cannot set 0th modulation
			modulation_ubo.send(pos, modulation);
		}

		void SpriteBatch::set_projection(const glm::vec4& projection_bounds) const
		{
			glm::mat3 proj = glm::ortho<float>(projection_bounds[0], projection_bounds[1], projection_bounds[2], projection_bounds[3]);
			glUseProgram(shader);
			glUniformMatrix3fv(projection_location, 1, GL_FALSE, glm::value_ptr(proj));
		}

		void SpriteBatch::set_global_modulation(const glm::vec4& modulation) const
		{
			glUseProgram(shader);
			glUniform4f(modulation_location, modulation[0], modulation[1], modulation[2], modulation[3]);
		}

		SpriteBatch::QuadReference::QuadReference(SpriteBatch* batch)
			: _batch(batch)
		{
			pos = _batch->pos_generator.gen();
			_info = &_batch->quad_info_ssbo.vector()[pos];
			_transform = &_batch->quad_transform_ssbo.vector()[pos];
			_batch->quads.push_back(this);
			_batch->dirty_z = true;
		}

		SpriteBatch::QuadReference::QuadReference(QuadReference&& other) noexcept
			: _batch(other._batch), pos(other.pos), _info(other._info), _transform(other._transform)
		{
			other.active = false;
			auto it = std::find(_batch->quads.begin(), _batch->quads.end(), &other);
			if (it != _batch->quads.end())
				*it = this;
		}

		SpriteBatch::QuadReference::~QuadReference()
		{
			if (active)
			{
				_batch->pos_generator.yield(pos);
				vector_erase(_batch->quads, this);
			}
		}

		SpriteBatch::QuadReference& SpriteBatch::QuadReference::operator=(QuadReference&& other) noexcept
		{
			if (this != &other)
			{
				if (active)
				{
					_batch->pos_generator.yield(pos);
					vector_erase(_batch->quads, this);
				}
				_batch = other._batch;
				other.active = false;
				auto it = std::find(_batch->quads.begin(), _batch->quads.end(), &other);
				if (it != _batch->quads.end())
					*it = this;
				pos = other.pos;
				_info = other._info;
				_transform = other._transform;
			}
			return *this;
		}

		void SpriteBatch::QuadReference::send_info() const
		{
			_batch->quad_info_ssbo.lazy_send(pos);
		}

		void SpriteBatch::QuadReference::send_transform() const
		{
			_batch->quad_transform_ssbo.lazy_send(pos);
		}

		void SpriteBatch::QuadReference::send_data() const
		{
			_batch->quad_info_ssbo.lazy_send(pos);
			_batch->quad_transform_ssbo.lazy_send(pos);
		}

		void SpriteBatch::swap_quad_order(QuadPos pos1, QuadPos pos2)
		{
			if (pos1 != pos2)
			{
				std::swap(ebo.vector()[pos1], ebo.vector()[pos2]);
				z_order.swap_range(pos1, pos2);
				ebo.lazy_send(pos1);
				ebo.lazy_send(pos2);
			}
		}

		void SpriteBatch::move_quad_order(QuadPos from, QuadPos to)
		{
			to = std::clamp(to, (QuadPos)0, QuadPos(capacity.quads - 1));
			from = std::clamp(from, (QuadPos)0, QuadPos(capacity.quads - 1));

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

		void SpriteBatch::flush()
		{
			for (renderable::Sprite* sprite : sprites)
				sprite->flush();
			quad_info_ssbo.flush();
			quad_transform_ssbo.flush();
			flush_z_values();
			ebo.flush();
		}

		void SpriteBatch::flush_z_values()
		{
			if (!dirty_z)
				return;
			dirty_z = false;
			for (QuadPos i = 0; i < quads.size() - 1; ++i)
			{
				bool swapped = false;
				for (QuadPos j = 0; j < quads.size() - i - 1; ++j)
				{
					if (quads[j]->z_value > quads[j + 1]->z_value)
					{
						std::swap(quads[j], quads[j + 1]);
						swap_quad_order(quads[j]->index_pos(), quads[j + 1]->index_pos());
						swapped = true;
					}
				}
				if (!swapped)
					break;
			}
		}
	}

	namespace renderable
	{
		Sprite::Sprite(batch::SpriteBatch* sprite_batch)
			: quad(sprite_batch)
		{
			batch().sprites.insert(this);
		}

		Sprite::Sprite(Sprite&& other) noexcept
			: quad(std::move(other.quad)), transformer(std::move(other.transformer))
		{
			batch().sprites.erase(&other);
		}

		Sprite::~Sprite()
		{
			batch().sprites.erase(this);
		}

		Sprite& Sprite::operator=(Sprite&& other) noexcept
		{
			if (this != &other)
			{
				bool different_batch = (&batch() != &other.batch());
				if (different_batch)
					batch().sprites.erase(this);
				quad = std::move(other.quad);
				transformer = std::move(other.transformer);
				batch().sprites.erase(&other);
				if (different_batch)
					batch().sprites.insert(this);
			}
			return *this;
		}

		void Sprite::post_set()
		{
			transformer.post_set();
		}

		void Sprite::pre_get() const
		{
			transformer.pre_get();
		}

		void Sprite::flush()
		{
			if (transformer.flush())
			{
				transformer.pre_get();
				quad.transform() = transformer.global();
				quad.send_transform();
			}
		}
	}
}
