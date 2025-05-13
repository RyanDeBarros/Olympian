#include "Text.h"

#include "core/base/Context.h"
#include "graphics/resources/Shaders.h"

namespace oly::rendering
{
	const TextBatch::GlyphInfo& TextBatch::get_glyph_info(GLuint vb_pos) const
	{
		return glyph_ssbo_block.get<INFO>(vb_pos);
	}

	TextBatch::GlyphInfo& TextBatch::set_glyph_info(GLuint vb_pos)
	{
		return glyph_ssbo_block.set<INFO>(vb_pos);
	}

	TextBatch::TextBatch(Capacity capacity)
		: ebo(vao, capacity.glyphs), tex_handles_ssbo(capacity.textures * sizeof(GLuint64)), vbo_block(vao, capacity.glyphs * 4), glyph_ssbo_block(capacity.glyphs), ubo(capacity.text_colors, capacity.modulations)
	{
		glm::ivec2 size = context::get_platform().window().get_size();
		projection_bounds = 0.5f * glm::vec4{ -size.x, size.x, -size.y, size.y };

		shader_locations.projection = glGetUniformLocation(graphics::internal_shaders::text_batch, "uProjection");
		shader_locations.modulation = glGetUniformLocation(graphics::internal_shaders::text_batch, "uGlobalModulation");

		ubo.text_color.send<TextColor>(0, {});
		ubo.modulation.send<ModulationRect>(0, {});

		vbo_block.attributes[VERTEX_POSITION] = graphics::VertexAttribute<float>{ 0, 2 };
		vbo_block.attributes[TEX_COORD] = graphics::VertexAttribute<float>{ 1, 2 };
		vbo_block.setup();
	}

	void TextBatch::render() const
	{
		vbo_block.pre_draw_all();
		glyph_ssbo_block.pre_draw_all();

		glBindVertexArray(vao);
		glUseProgram(graphics::internal_shaders::text_batch);
		glUniformMatrix3fv(shader_locations.projection, 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::ortho<float>(projection_bounds[0], projection_bounds[1], projection_bounds[2], projection_bounds[3]))));
		glUniform4f(shader_locations.modulation, global_modulation[0], global_modulation[1], global_modulation[2], global_modulation[3]);

		tex_handles_ssbo.bind_base(0);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, glyph_ssbo_block.buf.get_buffer<INFO>());
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, glyph_ssbo_block.buf.get_buffer<TRANSFORM>());
		ubo.text_color.bind_base(0);
		ubo.modulation.bind_base(1);
		ebo.render_elements(GL_TRIANGLES);

		vbo_block.post_draw_all();
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
		glyph_info_store.text_colors.decrement_usage(glyph_info.text_color_slot);
		glyph_info_store.modulations.decrement_usage(glyph_info.modulation_slot);
	}
		
	void TextBatch::set_texture(GLuint vb_pos, const graphics::BindlessTextureRes& texture)
	{
		if (glyph_info_store.textures.set_object<GLuint64>(tex_handles_ssbo, glyph_ssbo_block.buf.at<INFO>(vb_pos).tex_slot, vb_pos, texture, texture->get_handle()))
			glyph_ssbo_block.flag<INFO>(vb_pos);
	}
		
	void TextBatch::set_text_color(GLuint vb_pos, const TextColor& text_color)
	{
		if (glyph_info_store.text_colors.set_object(ubo.text_color, glyph_ssbo_block.buf.at<INFO>(vb_pos).text_color_slot, vb_pos, text_color))
			glyph_ssbo_block.flag<INFO>(vb_pos);
	}
		
	void TextBatch::set_modulation(GLuint vb_pos, const ModulationRect& modulation)
	{
		if (glyph_info_store.modulations.set_object(ubo.modulation, glyph_ssbo_block.buf.at<INFO>(vb_pos).modulation_slot, vb_pos, modulation))
			glyph_ssbo_block.flag<INFO>(vb_pos);
	}
		
	graphics::BindlessTextureRes TextBatch::get_texture(GLuint vb_pos) const
	{
		GLuint slot = get_glyph_info(vb_pos).tex_slot;
		return slot != 0 ? glyph_info_store.textures.get_object(slot) : nullptr;
	}
		
	TextBatch::TextColor TextBatch::get_text_color(GLuint vb_pos) const
	{
		GLuint slot = get_glyph_info(vb_pos).text_color_slot;
		return slot != 0 ? glyph_info_store.text_colors.get_object(slot) : TextColor{};
	}
		
	TextBatch::ModulationRect TextBatch::get_modulation(GLuint vb_pos) const
	{
		GLuint slot = get_glyph_info(vb_pos).modulation_slot;
		return slot != 0 ? glyph_info_store.modulations.get_object(slot) : ModulationRect{};
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
		
	void TextBatch::update_texture_handle(const graphics::BindlessTextureRes& texture)
	{
		GLuint slot;
		if (glyph_info_store.textures.get_slot({ texture }, slot))
			tex_handles_ssbo.send(slot, texture->get_handle());
	}
		
	TextGlyph::TextGlyph(TextBatch& text_batch)
		: batch(&text_batch)
	{
		vbid = batch->gen_glyph_id();
	}
		
	TextGlyph::TextGlyph(const TextGlyph& other)
		: batch(other.batch), transformer(other.transformer)
	{
		if (batch)
			vbid = batch->gen_glyph_id();
		else
			throw Error(ErrorCode::NULL_POINTER);

		set_texture(other.get_texture());
		set_tex_coords(other.get_tex_coords());
		set_text_color(other.get_text_color());
		set_modulation(other.get_modulation());
	}
		
	TextGlyph::TextGlyph(TextGlyph&& other) noexcept
		: batch(other.batch), vbid(std::move(other.vbid)), transformer(std::move(other.transformer))
	{
		other.batch = nullptr;
	}
		
	TextGlyph& TextGlyph::operator=(const TextGlyph& other)
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
			set_text_color(other.get_text_color());
			set_modulation(other.get_modulation());
		}
		return *this;
	}
		
	TextGlyph& TextGlyph::operator=(TextGlyph&& other) noexcept
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
		
	TextGlyph::~TextGlyph()
	{
		if (batch)
			batch->erase_glyph_id(vbid.get());
	}
		
	void TextGlyph::draw() const
	{
		if (transformer.flush())
		{
			transformer.pre_get();
			batch->glyph_ssbo_block.set<TextBatch::TRANSFORM>(vbid.get()) = transformer.global();
		}
		graphics::quad_indices(batch->ebo.draw_primitive().data(), vbid.get());
	}
		
	void TextGlyph::set_texture(const graphics::BindlessTextureRes& texture) const
	{
		batch->set_texture(vbid.get(), texture);
	}

	void TextGlyph::set_vertex_positions(const math::Rect2D& rect) const
	{
		batch->set_vertex_positions(vbid.get(), rect);
	}
		
	void TextGlyph::set_tex_coords(const math::Rect2D& rect) const
	{
		batch->set_tex_coords(vbid.get(), rect);
	}
		
	void TextGlyph::set_text_color(const TextBatch::TextColor& text_color) const
	{
		batch->set_text_color(vbid.get(), text_color);
	}
		
	void TextGlyph::set_modulation(const TextBatch::ModulationRect& modulation) const
	{
		batch->set_modulation(vbid.get(), modulation);
	}
		
	graphics::BindlessTextureRes TextGlyph::get_texture() const
	{
		return batch->get_texture(vbid.get());
	}
		
	math::Rect2D TextGlyph::get_vertex_positions() const
	{
		return batch->get_vertex_positions(vbid.get());
	}
		
	math::Rect2D TextGlyph::get_tex_coords() const
	{
		return batch->get_tex_coords(vbid.get());
	}
		
	TextBatch::TextColor TextGlyph::get_text_color() const
	{
		return batch->get_text_color(vbid.get());
	}
		
	TextBatch::ModulationRect TextGlyph::get_modulation() const
	{
		return batch->get_modulation(vbid.get());
	}
}
