#include "TextureQuads.h"

#include <glm/gtc/type_ptr.hpp>

#include <limits>

#include "Resources.h"

namespace oly
{
	namespace batch
	{
		TextureQuadBatch::Capacity::Capacity(GLushort quads, GLushort textures, GLushort uvs, GLushort modulations)
			: quads(quads), textures(textures), uvs(uvs), modulations(modulations)
		{
			OLY_ASSERT(4 * quads <= USHRT_MAX);
			OLY_ASSERT(textures > 0); // there is enough capacity for 0th texture
			OLY_ASSERT(0 < uvs && uvs <= 500);
			OLY_ASSERT(0 < modulations && modulations <= 250);
		}

		TextureQuadBatch::TextureQuadBatch(Capacity capacity, const glm::vec4& projection_bounds)
			: ebo(capacity.quads), capacity(capacity), tex_data_ssbo(capacity.textures), quad_info_ssbo(capacity.quads), quad_transform_ssbo(capacity.quads, 1.0f), tex_coords_ubo(capacity.uvs), modulation_ubo(capacity.modulations),
			z_order(capacity.quads), textures(capacity.textures)
		{
			shader = shaders::texture_quad_batch;
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

		void TextureQuadBatch::draw(size_t draw_spec)
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

		void TextureQuadBatch::set_texture(GLushort pos, const rendering::BindlessTextureRes& texture, rendering::ImageDimensions dim)
		{
			OLY_ASSERT(pos > 0 && pos < capacity.textures); // cannot set 0th texture
			textures[pos] = texture;
			texture->use_handle();
			TexData texture_data;
			texture_data.dimensions = { dim.w, dim.h };
			texture_data.handle = texture->get_handle();
			tex_data_ssbo.send(pos, texture_data);
		}

		void TextureQuadBatch::refresh_handle(GLushort pos, rendering::ImageDimensions dim)
		{
			OLY_ASSERT(pos > 0 && pos < capacity.textures); // cannot set 0th texture
			textures[pos]->use_handle();
			TexData texture_data;
			texture_data.dimensions = { dim.w, dim.h };
			texture_data.handle = textures[pos]->get_handle();
			tex_data_ssbo.send(pos, texture_data);
		}

		void TextureQuadBatch::refresh_handle(GLushort pos)
		{
			OLY_ASSERT(pos > 0 && pos < capacity.textures); // cannot set 0th texture
			textures[pos]->use_handle();
			GLuint64 handle = textures[pos]->get_handle();
			tex_data_ssbo.send(pos, &TexData::handle, handle);
		}

		void TextureQuadBatch::set_uvs(GLushort pos, const TexUVRect& tex_coords) const
		{
			OLY_ASSERT(pos > 0 && pos < capacity.uvs); // cannot set 0th UV
			tex_coords_ubo.send(pos, tex_coords);
		}

		void TextureQuadBatch::set_modulation(GLushort pos, const Modulation& modulation) const
		{
			OLY_ASSERT(pos > 0 && pos < capacity.modulations); // cannot set 0th modulation
			modulation_ubo.send(pos, modulation);
		}

		void TextureQuadBatch::set_projection(const glm::vec4& projection_bounds) const
		{
			glm::mat3 proj = glm::ortho<float>(projection_bounds[0], projection_bounds[1], projection_bounds[2], projection_bounds[3]);
			glUseProgram(shader);
			glUniformMatrix3fv(projection_location, 1, GL_FALSE, glm::value_ptr(proj));
		}

		void TextureQuadBatch::set_global_modulation(const glm::vec4& modulation) const
		{
			glUseProgram(shader);
			glUniform4f(modulation_location, modulation[0], modulation[1], modulation[2], modulation[3]);
		}

		TextureQuadBatch::QuadReference::QuadReference(TextureQuadBatch* batch)
			: _batch(batch)
		{
			pos = _batch->pos_generator.gen();
			_info = &_batch->quad_info_ssbo.vector()[pos];
			_transform = &_batch->quad_transform_ssbo.vector()[pos];
			_batch->quad_refs.push_back(this);
			_batch->dirty_z = true;
		}

		TextureQuadBatch::QuadReference::QuadReference(QuadReference&& other) noexcept
			: _batch(other._batch), pos(other.pos), _info(other._info), _transform(other._transform)
		{
			other.active = false;
			auto it = std::find(_batch->quad_refs.begin(), _batch->quad_refs.end(), &other);
			if (it != _batch->quad_refs.end())
				*it = this;
		}

		TextureQuadBatch::QuadReference::~QuadReference()
		{
			if (active)
			{
				_batch->pos_generator.yield(pos);
				_info->tex_slot = 0;
				send_info();
				vector_erase(_batch->quad_refs, this);
			}
		}

		TextureQuadBatch::QuadReference& TextureQuadBatch::QuadReference::operator=(QuadReference&& other) noexcept
		{
			if (this != &other)
			{
				if (active)
				{
					_batch->pos_generator.yield(pos);
					vector_erase(_batch->quad_refs, this);
				}
				_batch = other._batch;
				other.active = false;
				auto it = std::find(_batch->quad_refs.begin(), _batch->quad_refs.end(), &other);
				if (it != _batch->quad_refs.end())
					*it = this;
				pos = other.pos;
				_info = other._info;
				_transform = other._transform;
			}
			return *this;
		}

		void TextureQuadBatch::QuadReference::send_info() const
		{
			_batch->quad_info_ssbo.lazy_send(pos);
		}

		void TextureQuadBatch::QuadReference::send_transform() const
		{
			_batch->quad_transform_ssbo.lazy_send(pos);
		}

		void TextureQuadBatch::QuadReference::send_data() const
		{
			_batch->quad_info_ssbo.lazy_send(pos);
			_batch->quad_transform_ssbo.lazy_send(pos);
		}

		void TextureQuadBatch::swap_quad_order(QuadPos pos1, QuadPos pos2)
		{
			if (pos1 != pos2)
			{
				std::swap(ebo.vector()[pos1], ebo.vector()[pos2]);
				z_order.swap_range(pos1, pos2);
				ebo.lazy_send(pos1);
				ebo.lazy_send(pos2);
			}
		}

		void TextureQuadBatch::move_quad_order(QuadPos from, QuadPos to)
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

		void TextureQuadBatch::flush()
		{
			for (renderable::TextureQuad* texture_quad : texture_quads)
				texture_quad->flush();
			quad_info_ssbo.flush();
			quad_transform_ssbo.flush();
			flush_z_values();
			ebo.flush();
		}

		void TextureQuadBatch::flush_z_values()
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
	}

	namespace renderable
	{
		TextureQuad::TextureQuad(batch::TextureQuadBatch* sprite_batch)
			: quad(sprite_batch)
		{
			batch().texture_quads.insert(this);
		}

		TextureQuad::TextureQuad(TextureQuad&& other) noexcept
			: quad(std::move(other.quad)), transformer(std::move(other.transformer))
		{
			batch().texture_quads.erase(&other);
		}

		TextureQuad::~TextureQuad()
		{
			batch().texture_quads.erase(this);
		}

		TextureQuad& TextureQuad::operator=(TextureQuad&& other) noexcept
		{
			if (this != &other)
			{
				bool different_batch = (&batch() != &other.batch());
				if (different_batch)
					batch().texture_quads.erase(this);
				quad = std::move(other.quad);
				transformer = std::move(other.transformer);
				batch().texture_quads.erase(&other);
				if (different_batch)
					batch().texture_quads.insert(this);
			}
			return *this;
		}

		void TextureQuad::post_set()
		{
			transformer.post_set();
		}

		void TextureQuad::pre_get() const
		{
			transformer.pre_get();
		}

		void TextureQuad::flush()
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
