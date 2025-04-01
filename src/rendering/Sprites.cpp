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
			: ebo(capacity.quads), capacity(capacity), tex_data_ssbo(capacity.textures), quad_info_ssbo(capacity.quads), quad_transform_ssbo(capacity.quads, 1.0f), tex_coords_ubo(capacity.uvs), modulation_ubo(capacity.modulations),
			z_order(capacity.quads), quads(capacity.quads), textures(capacity.textures)
		{
			shader = shaders::sprite_batch;
			glUseProgram(shader);
			projection_location = glGetUniformLocation(shader, "uProjection");

			tex_coords_ubo.send(0, { { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } } });
			modulation_ubo.send(0, { { glm::vec4(1.0f), glm::vec4(1.0f), glm::vec4(1.0f), glm::vec4(1.0f) } });

			glBindVertexArray(vao);
			rendering::pre_init(ebo);
			ebo.init();

			set_projection(projection_bounds);

			glBindVertexArray(0);
		}

		void SpriteBatch::draw() const
		{
			glUseProgram(shader);
			glBindVertexArray(vao);

			tex_data_ssbo.bind_base(0);
			quad_info_ssbo.bind_base(1);
			quad_transform_ssbo.bind_base(2);
			tex_coords_ubo.bind_base(0);
			modulation_ubo.bind_base(1);
			ebo.draw(GL_TRIANGLES, GL_UNSIGNED_SHORT);
		}

		void SpriteBatch::set_texture(GLushort pos, const rendering::BindlessTextureRes& texture, rendering::ImageDimensions dim)
		{
			assert(pos > 0 && pos < capacity.textures); // cannot set 0th texture
			textures[pos] = texture;
			texture->use_handle();
			TexData texture_data;
			texture_data.dimensions = { dim.w, dim.h };
			texture_data.handle = texture->get_handle();
			tex_data_ssbo.send(pos, texture_data);
		}

		void SpriteBatch::refresh_handle(GLushort pos, rendering::ImageDimensions dim)
		{
			assert(pos > 0 && pos < capacity.textures); // cannot set 0th texture
			textures[pos]->use_handle();
			TexData texture_data;
			texture_data.dimensions = { dim.w, dim.h };
			texture_data.handle = textures[pos]->get_handle();
			tex_data_ssbo.send(pos, texture_data);
		}

		void SpriteBatch::refresh_handle(GLushort pos)
		{
			assert(pos > 0 && pos < capacity.textures); // cannot set 0th texture
			textures[pos]->use_handle();
			GLuint64 handle = textures[pos]->get_handle();
			tex_data_ssbo.send(pos, &TexData::handle, handle);
		}

		void SpriteBatch::set_uvs(GLushort pos, const TexUVRect& tex_coords) const
		{
			assert(pos > 0 && pos < capacity.uvs); // cannot set 0th UV
			tex_coords_ubo.send(pos, tex_coords);
		}

		void SpriteBatch::set_modulation(GLushort pos, const Modulation& modulation) const
		{
			assert(pos > 0 && pos < capacity.modulations); // cannot set 0th modulation
			modulation_ubo.send(pos, modulation);
		}

		void SpriteBatch::set_projection(const glm::vec4& projection_bounds) const
		{
			glm::mat3 proj = glm::ortho<float>(projection_bounds[0], projection_bounds[1], projection_bounds[2], projection_bounds[3]);
			glUseProgram(shader);
			glUniformMatrix3fv(projection_location, 1, GL_FALSE, glm::value_ptr(proj));
		}

		void SpriteBatch::QuadReference::send_info() const
		{
			_batch->quad_info_ssbo.lazy_send(_ssbo_pos);
		}

		void SpriteBatch::QuadReference::send_transform() const
		{
			_batch->quad_transform_ssbo.lazy_send(_ssbo_pos);
		}

		void SpriteBatch::QuadReference::send_data() const
		{
			_batch->quad_info_ssbo.lazy_send(_ssbo_pos);
			_batch->quad_transform_ssbo.lazy_send(_ssbo_pos);
		}

		SpriteBatch::QuadReference& SpriteBatch::get_quad(QuadPos pos)
		{
			QuadReference& quad = quads[pos];
			quad._info = &quad_info_ssbo.vector()[pos];
			quad._transform = &quad_transform_ssbo.vector()[pos];
			quad._batch = this;
			quad._ssbo_pos = pos;
			return quad;
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

		void SpriteBatch::flush() const
		{
			for (renderable::Sprite* sprite : sprites)
				sprite->flush();
			quad_info_ssbo.flush();
			quad_transform_ssbo.flush();
			ebo.flush();
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
			if (_quad)
				_quad->batch().sprites.insert(this);
		}

		Sprite::~Sprite()
		{
			if (_quad)
				_quad->batch().sprites.erase(this);
		}

		Sprite& Sprite::operator=(Sprite&& other) noexcept
		{
			if (this != &other)
			{
				if (_quad)
					_quad->batch().sprites.erase(this);
				_quad = other._quad;
				if (_quad)
					_quad->batch().sprites.insert(this);
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
