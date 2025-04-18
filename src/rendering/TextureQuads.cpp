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
			: ebo(capacity.quads), capacity(capacity), ssbo(capacity.textures, capacity.quads), ubo(capacity.uvs, capacity.modulations),
			z_order(capacity.quads), textures(capacity.textures)
		{
			shader_locations.projection = shaders::location(shaders::texture_quad_batch, "uProjection");
			shader_locations.modulation = shaders::location(shaders::texture_quad_batch, "uGlobalModulation");

			ubo.tex_coords.send<TexUVRect>(0, { { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } } });
			ubo.modulation.send<Modulation>(0, { { glm::vec4(1.0f), glm::vec4(1.0f), glm::vec4(1.0f), glm::vec4(1.0f) } });

			glBindVertexArray(vao);
			rendering::pre_init(ebo);
			ebo.bind();
			ebo.init();
			glBindVertexArray(0);

			set_projection(projection_bounds);
			set_global_modulation(glm::vec4(1.0f));
			draw_specs.push_back({ 0, capacity.quads });
		}

		void TextureQuadBatch::draw(size_t draw_spec)
		{
			glBindVertexArray(vao);
			glUseProgram(shaders::texture_quad_batch);

			ssbo.tex_data.bind_base(0);
			ssbo.quad_info.bind_base(1);
			ssbo.quad_transform.bind_base(2);
			ubo.tex_coords.bind_base(0);
			ubo.modulation.bind_base(1);
			ebo.set_draw_spec(draw_specs[draw_spec].initial, draw_specs[draw_spec].length);
			ebo.draw(GL_TRIANGLES, GL_UNSIGNED_SHORT);
		}

		void TextureQuadBatch::set_texture(GLushort pos, const rendering::BindlessTextureRes& texture, rendering::ImageDimensions dim)
		{
			OLY_ASSERT(pos > 0 && pos < capacity.textures); // cannot set 0th texture
			textures[pos] = texture;
			texture->use_handle();
			SSBO::TexData texture_data;
			texture_data.dimensions = { dim.w, dim.h };
			texture_data.handle = texture->get_handle();
			ssbo.tex_data.send(pos, texture_data);
		}

		void TextureQuadBatch::refresh_handle(GLushort pos, rendering::ImageDimensions dim)
		{
			OLY_ASSERT(pos > 0 && pos < capacity.textures); // cannot set 0th texture
			textures[pos]->use_handle();
			SSBO::TexData texture_data;
			texture_data.dimensions = { dim.w, dim.h };
			texture_data.handle = textures[pos]->get_handle();
			ssbo.tex_data.send(pos, texture_data);
		}

		void TextureQuadBatch::refresh_handle(GLushort pos)
		{
			OLY_ASSERT(pos > 0 && pos < capacity.textures); // cannot set 0th texture
			textures[pos]->use_handle();
			GLuint64 handle = textures[pos]->get_handle();
			ssbo.tex_data.send(pos, &SSBO::TexData::handle, handle);
		}

		void TextureQuadBatch::set_uvs(GLushort pos, const TexUVRect& tex_coords) const
		{
			OLY_ASSERT(pos > 0 && pos < capacity.uvs); // cannot set 0th UV
			ubo.tex_coords.send(pos, tex_coords);
		}

		void TextureQuadBatch::set_modulation(GLushort pos, const Modulation& modulation) const
		{
			OLY_ASSERT(pos > 0 && pos < capacity.modulations); // cannot set 0th modulation
			ubo.modulation.send(pos, modulation);
		}

		void TextureQuadBatch::set_projection(const glm::vec4& projection_bounds) const
		{
			glm::mat3 proj = glm::ortho<float>(projection_bounds[0], projection_bounds[1], projection_bounds[2], projection_bounds[3]);
			glUseProgram(shaders::texture_quad_batch);
			glUniformMatrix3fv(shader_locations.projection, 1, GL_FALSE, glm::value_ptr(proj));
		}

		void TextureQuadBatch::set_global_modulation(const glm::vec4& modulation) const
		{
			glUseProgram(shaders::texture_quad_batch);
			glUniform4f(shader_locations.modulation, modulation[0], modulation[1], modulation[2], modulation[3]);
		}

		TextureQuadBatch::QuadReference::QuadReference(TextureQuadBatch* batch)
			: _batch(batch)
		{
			pos = _batch->pos_generator.gen();
			_info = &_batch->ssbo.quad_info.vector()[pos];
			_transform = &_batch->ssbo.quad_transform.vector()[pos];
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
			_batch->ssbo.quad_info.lazy_send(pos);
		}

		void TextureQuadBatch::QuadReference::send_transform() const
		{
			_batch->ssbo.quad_transform.lazy_send(pos);
		}

		void TextureQuadBatch::QuadReference::send_data() const
		{
			_batch->ssbo.quad_info.lazy_send(pos);
			_batch->ssbo.quad_transform.lazy_send(pos);
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
			ssbo.quad_info.flush();
			ssbo.quad_transform.flush();
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
