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
			: ebo(capacity.quads), ubos(UBO::__UBO_COUNT), capacity(capacity), z_order(capacity.quads),
			quad_info_ssbo(capacity.quads), quad_transform_ssbo(capacity.quads, 1.0f), quads(capacity.quads), textures(capacity.textures)
		{
			shader = shaders::sprite_batch;
			glUseProgram(shader);
			projection_location = glGetUniformLocation(shader, "uProjection");

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, tex_data_ssbo);
			glNamedBufferStorage(tex_data_ssbo, capacity.textures * sizeof(TexData), nullptr, GL_DYNAMIC_STORAGE_BIT);

			glBindBuffer(GL_UNIFORM_BUFFER, ubos[UBO::B_TEX_COORDS]);
			glNamedBufferStorage(ubos[UBO::B_TEX_COORDS], capacity.uvs * sizeof(TexUVRect), nullptr, GL_DYNAMIC_STORAGE_BIT);
			TexUVRect tex_coords{ { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } } };
			glNamedBufferSubData(ubos[UBO::B_TEX_COORDS], 0, sizeof(TexUVRect), &tex_coords);

			glBindBuffer(GL_UNIFORM_BUFFER, ubos[UBO::B_MODULATION]);
			glNamedBufferStorage(ubos[UBO::B_MODULATION], capacity.modulations * sizeof(Modulation), nullptr, GL_DYNAMIC_STORAGE_BIT);
			Modulation modulation{ { glm::vec4(1.0f), glm::vec4(1.0f), glm::vec4(1.0f), glm::vec4(1.0f) } };
			glNamedBufferSubData(ubos[UBO::B_MODULATION], 0, sizeof(Modulation), &modulation);

			set_draw_spec(0, capacity.quads);

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

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, tex_data_ssbo);
			quad_info_ssbo.bind_base(1);
			quad_transform_ssbo.bind_base(2);
			glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubos[UBO::B_TEX_COORDS]);
			glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubos[UBO::B_MODULATION]);
			ebo.draw(GL_TRIANGLES, GL_UNSIGNED_SHORT);
		}

		void SpriteBatch::set_texture(size_t pos, const rendering::BindlessTextureRes& texture, rendering::ImageDimensions dim)
		{
			assert(pos > 0 && pos < capacity.textures); // cannot set 0th texture
			textures[pos] = texture;
			texture->use_handle();
			TexData texture_data;
			texture_data.dimensions = { dim.w, dim.h };
			texture_data.handle = texture->get_handle();
			glNamedBufferSubData(tex_data_ssbo, pos * sizeof(TexData), sizeof(TexData), &texture_data); // TODO move to process()
		}

		void SpriteBatch::refresh_handle(size_t pos, rendering::ImageDimensions dim)
		{
			assert(pos > 0 && pos < capacity.textures); // cannot set 0th texture
			textures[pos]->use_handle();
			TexData texture_data;
			texture_data.dimensions = { dim.w, dim.h };
			texture_data.handle = textures[pos]->get_handle();
			glNamedBufferSubData(tex_data_ssbo, pos * sizeof(TexData), sizeof(TexData), &texture_data); // TODO move to process()
		}

		void SpriteBatch::refresh_handle(size_t pos)
		{
			assert(pos > 0 && pos < capacity.textures); // cannot set 0th texture
			textures[pos]->use_handle();
			GLuint64 handle = textures[pos]->get_handle();
			glNamedBufferSubData(tex_data_ssbo, pos * sizeof(TexData) + offsetof(TexData, handle), sizeof(GLuint64), &handle); // TODO move to process()
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

		void SpriteBatch::Quad::send_info() const
		{
			_sprite_batch->quad_info_ssbo.lazy_send(_ssbo_pos);
		}

		void SpriteBatch::Quad::send_transform() const
		{
			_sprite_batch->quad_transform_ssbo.lazy_send(_ssbo_pos);
		}

		void SpriteBatch::Quad::send_data() const
		{
			_sprite_batch->quad_info_ssbo.lazy_send(_ssbo_pos);
			_sprite_batch->quad_transform_ssbo.lazy_send(_ssbo_pos);
		}

		SpriteBatch::Quad& SpriteBatch::get_quad(QuadPos pos)
		{
			Quad& quad = quads[pos];
			quad._info = &quad_info_ssbo.vector()[pos];
			quad._transform = &quad_transform_ssbo.vector()[pos];
			quad._sprite_batch = this;
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

		void SpriteBatch::process()
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
