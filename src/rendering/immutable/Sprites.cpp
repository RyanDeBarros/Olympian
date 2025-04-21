#include "Sprites.h"

#include <glm/gtc/type_ptr.hpp>

#include <limits>

#include "../Resources.h"

namespace oly
{
	namespace immut
	{
		SpriteBatch::Capacity::Capacity(GLushort quads, GLushort textures, GLushort uvs, GLushort modulations, GLushort gifs)
			: quads(quads), textures(textures + 1), uvs(uvs + 1), modulations(modulations + 1), gifs(gifs + 1)
		{
			OLY_ASSERT(4 * quads <= USHRT_MAX);
			OLY_ASSERT(uvs <= 500);
			OLY_ASSERT(modulations <= 250);
			OLY_ASSERT(gifs <= 1000);
		}

		SpriteBatch::SpriteBatch(Capacity capacity, const glm::vec4& projection_bounds)
			: ebo(capacity.quads), capacity(capacity), ssbo(capacity.textures, capacity.quads), ubo(capacity.uvs, capacity.modulations, capacity.gifs),
			z_order(capacity.quads), textures(capacity.textures), projection_bounds(projection_bounds)
		{
			shader_locations.projection = shaders::location(shaders::sprite_batch, "uProjection");
			shader_locations.modulation = shaders::location(shaders::sprite_batch, "uGlobalModulation");
			shader_locations.time = shaders::location(shaders::sprite_batch, "uTime");

			ubo.tex_coords.send<TexUVRect>(0, {});
			ubo.modulation.send<Modulation>(0, {});
			ubo.gif.send<rendering::GIFFrameFormat>(0, {});

			glBindVertexArray(vao);
			rendering::pre_init(ebo);
			ebo.bind();
			ebo.init();
			glBindVertexArray(0);

			draw_specs.push_back({ 0, capacity.quads });
		}

		void SpriteBatch::draw(size_t draw_spec)
		{
			glBindVertexArray(vao);
			glUseProgram(shaders::sprite_batch);
			glUniformMatrix3fv(shader_locations.projection, 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::ortho<float>(projection_bounds[0], projection_bounds[1], projection_bounds[2], projection_bounds[3]))));
			glUniform4f(shader_locations.modulation, global_modulation[0], global_modulation[1], global_modulation[2], global_modulation[3]);
			glUniform1f(shader_locations.time, TIME.now<float>());

			ssbo.tex_data.bind_base(0);
			ssbo.quad_info.bind_base(1);
			ssbo.quad_transform.bind_base(2);
			ubo.tex_coords.bind_base(0);
			ubo.modulation.bind_base(1);
			ubo.gif.bind_base(2);
			ebo.set_draw_spec(draw_specs[draw_spec].initial, draw_specs[draw_spec].length);
			ebo.draw(GL_TRIANGLES, GL_UNSIGNED_SHORT);
		}

		void SpriteBatch::set_texture(GLushort pos, const rendering::BindlessTextureRes& texture, rendering::ImageDimensions dim)
		{
			OLY_ASSERT(pos > 0 && pos < capacity.textures); // cannot set 0th texture
			textures[pos] = texture;
			texture->use_handle();
			SSBO::TexData texture_data;
			texture_data.dimensions = { dim.w, dim.h };
			texture_data.handle = texture->get_handle();
			ssbo.tex_data.send(pos, texture_data);
		}

		void SpriteBatch::refresh_handle(GLushort pos, rendering::ImageDimensions dim)
		{
			OLY_ASSERT(pos > 0 && pos < capacity.textures); // cannot set 0th texture
			textures[pos]->use_handle();
			SSBO::TexData texture_data;
			texture_data.dimensions = { dim.w, dim.h };
			texture_data.handle = textures[pos]->get_handle();
			ssbo.tex_data.send(pos, texture_data);
		}

		void SpriteBatch::refresh_handle(GLushort pos)
		{
			OLY_ASSERT(pos > 0 && pos < capacity.textures); // cannot set 0th texture
			textures[pos]->use_handle();
			GLuint64 handle = textures[pos]->get_handle();
			ssbo.tex_data.send(pos, &SSBO::TexData::handle, handle);
		}

		void SpriteBatch::set_uvs(GLushort pos, const TexUVRect& tex_coords) const
		{
			OLY_ASSERT(pos > 0 && pos < capacity.uvs); // cannot set 0th UV
			ubo.tex_coords.send(pos, tex_coords);
		}

		void SpriteBatch::set_modulation(GLushort pos, const Modulation& modulation) const
		{
			OLY_ASSERT(pos > 0 && pos < capacity.modulations); // cannot set 0th modulation
			ubo.modulation.send(pos, modulation);
		}

		void SpriteBatch::set_frame_format(GLushort pos, const rendering::GIFFrameFormat& gif) const
		{
			OLY_ASSERT(pos < capacity.gifs);
			ubo.gif.send(pos, gif);
		}

		SpriteBatch::QuadReference::QuadReference(SpriteBatch* batch)
			: _batch(batch)
		{
			pos = _batch->pos_generator.generate();
			_info = &_batch->ssbo.quad_info.vector()[pos.get()];
			_transform = &_batch->ssbo.quad_transform.vector()[pos.get()];
			_batch->quad_refs.push_back(this);
			_batch->dirty_z = true;
		}

		SpriteBatch::QuadReference::QuadReference(QuadReference&& other) noexcept
			: _batch(other._batch), pos(std::move(other.pos)), _info(other._info), _transform(other._transform)
		{
			other.active = false;
			auto it = std::find(_batch->quad_refs.begin(), _batch->quad_refs.end(), &other);
			if (it != _batch->quad_refs.end())
				*it = this;
		}

		SpriteBatch::QuadReference::~QuadReference()
		{
			if (active)
			{
				_info->tex_slot = 0;
				send_info();
				vector_erase(_batch->quad_refs, this);
			}
		}

		SpriteBatch::QuadReference& SpriteBatch::QuadReference::operator=(QuadReference&& other) noexcept
		{
			if (this != &other)
			{
				if (active)
					vector_erase(_batch->quad_refs, this);
				_batch = other._batch;
				other.active = false;
				auto it = std::find(_batch->quad_refs.begin(), _batch->quad_refs.end(), &other);
				if (it != _batch->quad_refs.end())
					*it = this;
				pos = std::move(other.pos);
				_info = other._info;
				_transform = other._transform;
			}
			return *this;
		}

		void SpriteBatch::QuadReference::send_info() const
		{
			_batch->ssbo.quad_info.lazy_send(pos.get());
		}

		void SpriteBatch::QuadReference::send_transform() const
		{
			_batch->ssbo.quad_transform.lazy_send(pos.get());
		}

		void SpriteBatch::QuadReference::send_data() const
		{
			_batch->ssbo.quad_info.lazy_send(pos.get());
			_batch->ssbo.quad_transform.lazy_send(pos.get());
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
			for (Sprite* sprite : sprites)
				sprite->flush();
			ssbo.quad_info.flush();
			ssbo.quad_transform.flush();
			flush_z_values();
			ebo.flush();
		}

		void SpriteBatch::flush_z_values()
		{
			if (!dirty_z)
				return;
			dirty_z = false;
			for (QuadPos i = 0; i < quad_refs.size() - 1; ++i)
			{
				bool swapped = false;
				for (QuadPos j = 0; j < quad_refs.size() - i - 1; ++j)
				{
					if (quad_refs[j]->z_value > quad_refs[j + 1]->z_value)
					{
						std::swap(quad_refs[j], quad_refs[j + 1]);
						swap_quad_order(quad_refs[j]->index_pos(), quad_refs[j + 1]->index_pos());
						swapped = true;
					}
				}
				if (!swapped)
					break;
			}
		}

		Sprite::Sprite(SpriteBatch* sprite_batch)
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
