#pragma once

#include "core/base/Transforms.h"
#include "core/base/Constants.h"
#include "core/math/Shapes.h"
#include "core/containers/IDGenerator.h"

#include "graphics/BatchBarrier.h"
#include "graphics/backend/basic/Textures.h"
#include "graphics/backend/specialized/ElementBuffers.h"
#include "graphics/backend/specialized/VertexBuffers.h"
#include "graphics/backend/specialized/LightweightBuffers.h"
#include "graphics/backend/specialized/UsageSlotTracker.h"

namespace oly::rendering
{
	struct TextGlyph;
	class TextBatch
	{
		friend struct TextGlyph;

		graphics::VertexArray vao;
		graphics::PersistentEBO<6> ebo;

		enum
		{
			VERTEX_POSITION,
			TEX_COORD
		};
		graphics::PersistentVertexBufferBlock<glm::vec2, glm::vec2> vbo_block;

		graphics::LightweightSSBO<graphics::Mutability::MUTABLE> tex_handles_ssbo;

		struct GlyphInfo
		{
			GLuint tex_slot;
			GLuint modulation_slot;
		};

		enum
		{
			INFO,
			TRANSFORM
		};
		graphics::LazyPersistentGPUBufferBlock<GlyphInfo, glm::mat3> glyph_ssbo_block;

		const GlyphInfo& get_glyph_info(GLuint vb_pos) const;
		GlyphInfo& set_glyph_info(GLuint vb_pos);

		struct
		{
			GLuint projection, modulation;
		} shader_locations;

		static const GLuint max_modulations = 1000;

		struct UBO
		{
			graphics::LightweightUBO<graphics::Mutability::MUTABLE> modulation;

			UBO(GLuint modulations)
				: modulation(modulations * sizeof(glm::vec4), max_modulations * sizeof(glm::vec4)) {}
		} ubo;

	public:
		glm::mat3 projection = 1.0f;
		glm::vec4 global_modulation = glm::vec4(1.0f);

		struct Capacity
		{
			Capacity(GLuint initial_glyphs = 1, GLuint new_textures = 0, GLuint new_modulations = 0)
				: glyphs(initial_glyphs), textures(new_textures + 1), modulations(new_modulations + 1)
			{
				OLY_ASSERT(4 * initial_glyphs <= nmax<unsigned int>());
				OLY_ASSERT(modulations <= max_modulations);
			}

		private:
			friend class TextBatch;
			GLuint glyphs, textures, modulations;
		};

		TextBatch(Capacity capacity = {});
		TextBatch(const TextBatch&) = delete;
		TextBatch(TextBatch&&) = delete;

		void render() const;

	private:
		StrictIDGenerator<GLuint> vbid_generator;
		typedef StrictIDGenerator<GLuint>::ID VBID;
		VBID gen_glyph_id();
		void erase_glyph_id(const VBID& id);

		struct GlyphInfoStore
		{
			graphics::UsageSlotTracker<graphics::BindlessTextureRef> textures;
			graphics::UsageSlotTracker<glm::vec4> modulations;
		} glyph_info_store;

		void set_texture(GLuint vb_pos, const graphics::BindlessTextureRef& texture);
		void set_text_color(GLuint vb_pos, const glm::vec4 text_color);

		graphics::BindlessTextureRef get_texture(GLuint vb_pos) const;
		glm::vec4 get_text_color(GLuint vb_pos) const;

		void set_vertex_positions(GLuint vb_pos, const math::Rect2D& rect);
		void set_tex_coords(GLuint vb_pos, const math::Rect2D& rect);

		math::Rect2D get_vertex_positions(GLuint vb_pos) const;
		math::Rect2D get_tex_coords(GLuint vb_pos) const;

	public:
		void update_texture_handle(const graphics::BindlessTextureRef& texture);
	};

	constexpr TextBatch* CONTEXT_TEXT_BATCH = nullptr;

	struct TextGlyph
	{
	private:
		friend class TextBatch;
		TextBatch& batch;
		const bool in_context;
		TextBatch::VBID vbid;

	public:
		Transformer2D transformer;

		TextGlyph(TextBatch* batch = CONTEXT_TEXT_BATCH);
		TextGlyph(const TextGlyph&);
		TextGlyph(TextGlyph&&) noexcept;
		TextGlyph& operator=(const TextGlyph&);
		TextGlyph& operator=(TextGlyph&&) noexcept;
		~TextGlyph();

		void draw(BatchBarrier barrier = batch::BARRIER) const;

		void set_texture(const graphics::BindlessTextureRef& texture) const;
		void set_vertex_positions(const math::Rect2D& rect) const;
		void set_tex_coords(const math::Rect2D& rect) const;
		void set_text_color(glm::vec4 text_color) const;

		graphics::BindlessTextureRef get_texture() const;
		math::Rect2D get_vertex_positions() const;
		math::Rect2D get_tex_coords() const;
		glm::vec4 get_text_color() const;

		const Transform2D& get_local() const { return transformer.get_local(); }
		Transform2D& set_local() { return transformer.set_local(); }
	};
}
