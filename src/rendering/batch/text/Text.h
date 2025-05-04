#pragma once

#include "core/base/Transforms.h"
#include "core/math/Geometry.h"
#include "core/containers/IDGenerator.h"

#include "graphics/backend/basic/Textures.h"
#include "graphics/backend/specialized/General.h"

#include "../UsageSlotTracker.h"

namespace oly
{
	namespace rendering
	{
		struct TextGlyph;
		class TextBatch
		{
			friend struct TextGlyph;

			VertexArray vao;
			PersistentEBO<> ebo;

			enum
			{
				VERTEX_POSITION,
				TEX_COORD
			};
			PersistentVertexBufferBlock<glm::vec2, glm::vec2> vbo_block;

			struct GlyphInfo
			{
				GLuint tex_slot;
				GLuint text_color_slot;
				GLuint modulation_slot;
			};

			LightweightSSBO<Mutability::MUTABLE> tex_handles_ssbo;
			enum
			{
				INFO,
				TRANSFORM
			};
			LazyPersistentGPUBufferBlock<GlyphInfo, glm::mat3> glyph_ssbo_block;

			const GlyphInfo& get_glyph_info(GLuint vb_pos) const;
			GlyphInfo& set_glyph_info(GLuint vb_pos);

			struct
			{
				GLuint projection, modulation;
			} shader_locations;

		public:
			struct TextColor
			{
				glm::vec4 color = glm::vec4(1.0f);

				bool operator==(const TextColor&) const = default;
			};
			struct TextColorHash
			{
				size_t operator()(const TextColor& c) const {
					return std::hash<glm::vec4>{}(c.color);
				}
			};
			struct Modulation
			{
				glm::vec4 colors[4] = { glm::vec4(1.0f), glm::vec4(1.0f), glm::vec4(1.0f), glm::vec4(1.0f) };

				bool operator==(const Modulation&) const = default;
			};
			struct ModulationHash
			{
				size_t operator()(const Modulation& mod) const {
					return std::hash<glm::vec4>{}(mod.colors[0]) ^ (std::hash<glm::vec4>{}(mod.colors[1]) << 1) ^ (std::hash<glm::vec4>{}(mod.colors[2]) << 2) ^ (std::hash<glm::vec4>{}(mod.colors[3]) << 3);
				}
			};

		private:
			struct UBO
			{
				LightweightUBO<Mutability::MUTABLE> text_color, modulation;

				UBO(GLuint text_colors, GLuint modulations)
					: text_color(text_colors * sizeof(TextColor)), modulation(modulations * sizeof(Modulation)) {}
			} ubo;

		public:
			glm::vec4 projection_bounds, global_modulation = glm::vec4(1.0f);

			struct Capacity
			{
				Capacity(GLuint initial_glyphs, GLuint new_textures, GLuint new_text_colors = 0, GLuint new_modulations = 0)
					: glyphs(initial_glyphs), textures(new_textures + 1), text_colors(new_text_colors + 1), modulations(new_modulations + 1)
				{
					// TODO there seems to be an issue if the initial_glyphs passed is 0. The process gets stuck in TextGlyph constructor.
					OLY_ASSERT(4 * initial_glyphs <= UINT_MAX);
					OLY_ASSERT(text_colors <= 1000); // TODO here and elsewhere, enforce this constraint when resizing -> add max_size to mutable specialized buffers
					OLY_ASSERT(modulations <= 250);
				}

			private:
				friend class TextBatch;
				GLuint glyphs, textures, text_colors, modulations;
			};

			TextBatch(Capacity capacity, const glm::vec4& projection_bounds);

			void render() const;

		private:
			StrictIDGenerator<GLuint> vbid_generator;
			typedef StrictIDGenerator<GLuint>::ID VBID;
			VBID gen_glyph_id();
			void erase_glyph_id(GLuint id);

			struct GlyphInfoStore
			{
				UsageSlotTracker<BindlessTextureRes> textures;
				UsageSlotTracker<TextColor, TextColorHash> text_colors;
				UsageSlotTracker<Modulation, ModulationHash> modulations;
			} glyph_info_store;

			void set_texture(GLuint vb_pos, const BindlessTextureRes& texture);
			void set_text_color(GLuint vb_pos, const TextColor& text_color);
			void set_modulation(GLuint vb_pos, const Modulation& modulation);

			BindlessTextureRes get_texture(GLuint vb_pos) const;
			TextColor get_text_color(GLuint vb_pos) const;
			Modulation get_modulation(GLuint vb_pos) const;

			void set_vertex_positions(GLuint vb_pos, const math::Rect2D& rect);
			void set_tex_coords(GLuint vb_pos, const math::Rect2D& rect);

			math::Rect2D get_vertex_positions(GLuint vb_pos) const;
			math::Rect2D get_tex_coords(GLuint vb_pos) const;

		public:
			void update_texture_handle(const BindlessTextureRes& texture);
		};

		struct TextGlyph
		{
		private:
			friend class TextBatch;
			TextBatch* batch;
			TextBatch::VBID vbid;

		public:
			Transformer2D transformer;

			TextGlyph(TextBatch& text_batch);
			TextGlyph(const TextGlyph&);
			TextGlyph(TextGlyph&&) noexcept;
			TextGlyph& operator=(const TextGlyph&);
			TextGlyph& operator=(TextGlyph&&) noexcept;
			~TextGlyph();

			void draw() const;

			void set_texture(const BindlessTextureRes& texture) const;
			void set_vertex_positions(const math::Rect2D& rect) const;
			void set_tex_coords(const math::Rect2D& rect) const;
			void set_text_color(const TextBatch::TextColor& text_color) const;
			void set_modulation(const TextBatch::Modulation& modulation) const;

			BindlessTextureRes get_texture() const;
			math::Rect2D get_vertex_positions() const;
			math::Rect2D get_tex_coords() const;
			TextBatch::TextColor get_text_color() const;
			TextBatch::Modulation get_modulation() const;

			const TextBatch& get_batch() const { return *batch; }
			TextBatch& get_batch() { return *batch; }
			const Transform2D& get_local() const { return transformer.get_local(); }
			Transform2D& set_local() { return transformer.set_local(); }
		};
	}
}
