#include "Text.h"

#include <glm/gtc/type_ptr.hpp>

#include "../../Resources.h"

namespace oly
{
	namespace rendering
	{
		const TextBatch::GlyphInfo& TextBatch::get_glyph_info(GLuint vb_pos) const
		{
			return glyph_ssbo_block.get<INFO>(vb_pos);
		}

		TextBatch::GlyphInfo& TextBatch::set_glyph_info(GLuint vb_pos)
		{
			return glyph_ssbo_block.set<INFO>(vb_pos);
		}

		TextBatch::TextBatch(Capacity capacity, const glm::vec4& projection_bounds)
			: ebo(vao, capacity.glyphs), tex_handles_ssbo(capacity.textures * sizeof(GLuint64)), vbo_block(vao, capacity.glyphs * 4), glyph_ssbo_block(capacity.glyphs),
			ubo(capacity.foregrounds, capacity.backgrounds, capacity.modulations), projection_bounds(projection_bounds)
		{
			shader_locations.projection = shaders::location(shaders::sprite_batch, "uProjection");
			shader_locations.modulation = shaders::location(shaders::sprite_batch, "uGlobalModulation");

			ubo.foreground.send<Foreground>(0, {});
			ubo.background.send<Background>(0, {});
			ubo.modulation.send<Modulation>(0, {});

			vbo_block.attributes[VERTEX_POSITION] = VertexAttribute<float>{ 0, 2 };
			vbo_block.attributes[TEX_COORD] = VertexAttribute<float>{ 1, 2 };
			vbo_block.setup();
		}

		void TextBatch::render() const
		{
			glyph_ssbo_block.pre_draw_all();

			glBindVertexArray(vao);
			glUseProgram(shaders::text_batch);
			glUniformMatrix3fv(shader_locations.projection, 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::ortho<float>(projection_bounds[0], projection_bounds[1], projection_bounds[2], projection_bounds[3]))));
			glUniform4f(shader_locations.modulation, global_modulation[0], global_modulation[1], global_modulation[2], global_modulation[3]);

			tex_handles_ssbo.bind_base(0);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, glyph_ssbo_block.buf.get_buffer<INFO>());
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, glyph_ssbo_block.buf.get_buffer<TRANSFORM>());
			ubo.foreground.bind_base(0);
			ubo.background.bind_base(1);
			ubo.modulation.bind_base(2);
			ebo.render_elements(GL_TRIANGLES);

			glyph_ssbo_block.post_draw_all();
		}
		
		TextBatch::VBID TextBatch::gen_glyph_id()
		{
			VBID id = vbid_generator.generate();
			glyph_ssbo_block.set<INFO>(id.get()) = {};
			glyph_ssbo_block.set<TRANSFORM>(id.get()) = 1.0f;
			return id;
		}
		
		void TextBatch::erase_glyph_id(GLuint id)
		{
			const GlyphInfo& glyph_info = glyph_ssbo_block.buf.at<INFO>(id);
			glyph_info_store.textures.decrement_usage(glyph_info.tex_slot);
			glyph_info_store.foregrounds.decrement_usage(glyph_info.foreground_color_slot);
			glyph_info_store.backgrounds.decrement_usage(glyph_info.background_color_slot);
			glyph_info_store.modulations.decrement_usage(glyph_info.modulation_color_slot);
		}
		
		void TextBatch::set_texture(GLuint vb_pos, const BindlessTextureRes& texture)
		{
			if (glyph_info_store.textures.set_object<GLuint64>(tex_handles_ssbo, glyph_ssbo_block.buf.at<INFO>(vb_pos).tex_slot, vb_pos, texture, texture->get_handle()))
				glyph_ssbo_block.flag<INFO>(vb_pos);
		}
		
		void TextBatch::set_foreground(GLuint vb_pos, const Foreground& foreground)
		{
			if (glyph_info_store.foregrounds.set_object(ubo.foreground, glyph_ssbo_block.buf.at<INFO>(vb_pos).foreground_color_slot, vb_pos, foreground))
				glyph_ssbo_block.flag<INFO>(vb_pos);
		}
		
		void TextBatch::set_background(GLuint vb_pos, const Background& background)
		{
			if (glyph_info_store.backgrounds.set_object(ubo.background, glyph_ssbo_block.buf.at<INFO>(vb_pos).background_color_slot, vb_pos, background))
				glyph_ssbo_block.flag<INFO>(vb_pos);
		}
		
		void TextBatch::set_modulation(GLuint vb_pos, const Modulation& modulation)
		{
			if (glyph_info_store.modulations.set_object(ubo.modulation, glyph_ssbo_block.buf.at<INFO>(vb_pos).modulation_color_slot, vb_pos, modulation))
				glyph_ssbo_block.flag<INFO>(vb_pos);
		}
		
		BindlessTextureRes TextBatch::get_texture(GLuint vb_pos) const
		{
			GLuint slot = get_glyph_info(vb_pos).tex_slot;
			return slot != 0 ? glyph_info_store.textures.get_object(slot) : nullptr;
		}
		
		TextBatch::Foreground TextBatch::get_foreground(GLuint vb_pos) const
		{
			GLuint slot = get_glyph_info(vb_pos).foreground_color_slot;
			return slot != 0 ? glyph_info_store.foregrounds.get_object(slot) : Foreground{};
		}
		
		TextBatch::Background TextBatch::get_background(GLuint vb_pos) const
		{
			GLuint slot = get_glyph_info(vb_pos).background_color_slot;
			return slot != 0 ? glyph_info_store.backgrounds.get_object(slot) : Background{};
		}
		
		TextBatch::Modulation TextBatch::get_modulation(GLuint vb_pos) const
		{
			GLuint slot = get_glyph_info(vb_pos).modulation_color_slot;
			return slot != 0 ? glyph_info_store.modulations.get_object(slot) : Modulation{};
		}

		void TextBatch::set_vertex_positions(GLuint vb_pos, const math::Rect2D& rect)
		{
			glm::vec2* vertex_positions = vbo_block.set<VERTEX_POSITION>(4 * vb_pos, 4);
			auto uvs = rect.uvs();
			vertex_positions[0] = uvs[0];
			vertex_positions[1] = uvs[1];
			vertex_positions[2] = uvs[2];
			vertex_positions[3] = uvs[3];
		}

		void TextBatch::set_tex_coords(GLuint vb_pos, const math::Rect2D& rect)
		{
			glm::vec2* tex_coords = vbo_block.set<TEX_COORD>(4 * vb_pos, 4);
			auto uvs = rect.uvs();
			tex_coords[0] = uvs[0];
			tex_coords[1] = uvs[1];
			tex_coords[2] = uvs[2];
			tex_coords[3] = uvs[3];
		}

		math::Rect2D TextBatch::get_vertex_positions(GLuint vb_pos) const
		{
			glm::vec2 bl = vbo_block.get<VERTEX_POSITION>(4 * vb_pos);
			glm::vec2 tr = vbo_block.get<VERTEX_POSITION>(4 * vb_pos + 2);
			return { bl.x, tr.x, bl.y, tr.y };
		}

		math::Rect2D TextBatch::get_tex_coords(GLuint vb_pos) const
		{
			glm::vec2 bl = vbo_block.get<TEX_COORD>(4 * vb_pos);
			glm::vec2 tr = vbo_block.get<TEX_COORD>(4 * vb_pos + 2);
			return { bl.x, tr.x, bl.y, tr.y };
		}
		
		void TextBatch::update_texture_handle(const BindlessTextureRes& texture)
		{
			GLuint slot;
			if (glyph_info_store.textures.get_slot({ texture }, slot))
				tex_handles_ssbo.send(slot, texture->get_handle());
		}
		
		GlyphText::GlyphText(TextBatch& text_batch)
			: batch(&text_batch)
		{
			vbid = batch->gen_glyph_id();
		}
		
		GlyphText::GlyphText(const GlyphText& other)
			: batch(other.batch), transformer(other.transformer)
		{
			if (batch)
				vbid = batch->gen_glyph_id();
			else
				throw Error(ErrorCode::NULL_POINTER);

			set_texture(other.get_texture());
			set_tex_coords(other.get_tex_coords());
			set_foreground(other.get_foreground());
			set_background(other.get_background());
			set_modulation(other.get_modulation());
		}
		
		GlyphText::GlyphText(GlyphText&& other) noexcept
			: batch(other.batch), vbid(std::move(other.vbid)), transformer(std::move(other.transformer))
		{
			other.batch = nullptr;
		}
		
		GlyphText& GlyphText::operator=(const GlyphText& other)
		{
			if (this != &other)
			{
				if (batch != other.batch)
				{
					if (batch)
						batch->erase_glyph_id(vbid.get());
					batch = other.batch;
					if (batch)
						vbid = batch->gen_glyph_id();
					else
						throw Error(ErrorCode::NULL_POINTER);
				}
				transformer = other.transformer;

				set_texture(other.get_texture());
				set_tex_coords(other.get_tex_coords());
				set_foreground(other.get_foreground());
				set_background(other.get_background());
				set_modulation(other.get_modulation());
			}
			return *this;
		}
		
		GlyphText& GlyphText::operator=(GlyphText&& other) noexcept
		{
			if (this != &other)
			{
				if (batch)
					batch->erase_glyph_id(vbid.get());
				batch = other.batch;
				vbid = std::move(other.vbid);
				transformer = std::move(other.transformer);
				other.batch = nullptr;
			}
			return *this;
		}
		
		GlyphText::~GlyphText()
		{
			if (batch)
				batch->erase_glyph_id(vbid.get());
		}
		
		void GlyphText::draw() const
		{
			if (transformer.flush())
			{
				transformer.pre_get();
				batch->glyph_ssbo_block.set<TextBatch::TRANSFORM>(vbid.get()) = transformer.global();
			}
			quad_indices(batch->ebo.draw_primitive().data(), vbid.get());
		}
		
		void GlyphText::set_texture(const BindlessTextureRes& texture) const
		{
			batch->set_texture(vbid.get(), texture);
		}

		void GlyphText::set_vertex_positions(const math::Rect2D& rect) const
		{
			batch->set_vertex_positions(vbid.get(), rect);
		}
		
		void GlyphText::set_tex_coords(const math::Rect2D& rect) const
		{
			batch->set_tex_coords(vbid.get(), rect);
		}
		
		void GlyphText::set_foreground(const TextBatch::Foreground& foreground) const
		{
			batch->set_foreground(vbid.get(), foreground);
		}
		
		void GlyphText::set_background(const TextBatch::Background& background) const
		{
			batch->set_background(vbid.get(), background);
		}
		
		void GlyphText::set_modulation(const TextBatch::Modulation& modulation) const
		{
			batch->set_modulation(vbid.get(), modulation);
		}
		
		BindlessTextureRes GlyphText::get_texture() const
		{
			return batch->get_texture(vbid.get());
		}
		
		math::Rect2D GlyphText::get_vertex_positions() const
		{
			return batch->get_vertex_positions(vbid.get());
		}
		
		math::Rect2D GlyphText::get_tex_coords() const
		{
			return batch->get_tex_coords(vbid.get());
		}
		
		TextBatch::Foreground GlyphText::get_foreground() const
		{
			return batch->get_foreground(vbid.get());
		}
		
		TextBatch::Background GlyphText::get_background() const
		{
			return batch->get_background(vbid.get());
		}
		
		TextBatch::Modulation GlyphText::get_modulation() const
		{
			return batch->get_modulation(vbid.get());
		}
	}
}
