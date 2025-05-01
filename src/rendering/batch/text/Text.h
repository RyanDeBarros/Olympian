#pragma once

#include "../../SpecializedBuffers.h"
#include "../UsageSlotTracker.h"
#include "util/IDGenerator.h"
#include "math/Transforms.h"
#include "math/Geometry.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

namespace oly
{
	namespace rendering
	{
		struct GlyphText;
		class TextBatch
		{
			friend struct GlyphText;

			VertexArray vao;
			PersistentEBO<> ebo;

			enum
			{
				VERTEX_POSITION,
				TEX_COORD
			};
			LazyPersistentGPUBufferBlock<glm::vec2, glm::vec2> vbo_block;

			struct GlyphInfo
			{
				GLuint tex_slot;
				GLuint foreground_color_slot;
				GLuint background_color_slot;
				GLuint modulation_color_slot;
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
			struct Foreground
			{
				glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };

				bool operator==(const Foreground&) const = default;
			};
			struct ForegroundHash
			{
				size_t operator()(const Foreground& foreground) const {
					return std::hash<glm::vec4>{}(foreground.color);
				}
			};
			struct Background
			{
				glm::vec4 color = { 0.0f, 0.0f, 0.0f, 0.0f };

				bool operator==(const Background&) const = default;
			};
			struct BackgroundHash
			{
				size_t operator()(const Background& background) const {
					return std::hash<glm::vec4>{}(background.color);
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
				LightweightUBO<Mutability::MUTABLE> foreground, background, modulation;

				UBO(GLuint foregrounds, GLuint backgrounds, GLuint modulations)
					: foreground(foregrounds * sizeof(glm::vec4)), background(backgrounds * sizeof(glm::vec4)), modulation(modulations * sizeof(Modulation)) {}
			} ubo;

		public:
			glm::vec4 projection_bounds, global_modulation = glm::vec4(1.0f);

			struct Capacity
			{
				Capacity(GLuint initial_glyphs, GLuint new_textures, GLuint new_foregrounds = 0, GLuint new_backgrounds = 0, GLuint new_modulations = 0)
					: glyphs(initial_glyphs), textures(new_textures + 1), foregrounds(new_foregrounds + 1), backgrounds(new_backgrounds + 1), modulations(new_modulations + 1)
				{
					OLY_ASSERT(4 * initial_glyphs <= UINT_MAX);
					OLY_ASSERT(foregrounds <= 1000);
					OLY_ASSERT(backgrounds <= 1000); // TODO here and elsewhere, enforce this constraint when resizing
					OLY_ASSERT(modulations <= 250);
				}

			private:
				friend class TextBatch;
				GLuint glyphs, textures, foregrounds, backgrounds, modulations;
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
				UsageSlotTracker<Foreground, ForegroundHash> foregrounds;
				UsageSlotTracker<Background, BackgroundHash> backgrounds;
				UsageSlotTracker<Modulation, ModulationHash> modulations;
			} glyph_info_store;

			void set_texture(GLuint vb_pos, const BindlessTextureRes& texture);
			void set_foreground(GLuint vb_pos, const Foreground& foreground);
			void set_background(GLuint vb_pos, const Background& background);
			void set_modulation(GLuint vb_pos, const Modulation& modulation);

			BindlessTextureRes get_texture(GLuint vb_pos) const;
			Foreground get_foreground(GLuint vb_pos) const;
			Background get_background(GLuint vb_pos) const;
			Modulation get_modulation(GLuint vb_pos) const;

			void set_vertex_positions(GLuint vb_pos, const math::Rect2D& rect);
			void set_tex_coords(GLuint vb_pos, const math::Rect2D& rect);

			math::Rect2D get_vertex_positions(GLuint vb_pos) const;
			math::Rect2D get_tex_coords(GLuint vb_pos) const;

		public:
			void update_texture_handle(const BindlessTextureRes& texture);
		};

		struct GlyphText
		{
		private:
			friend class TextBatch;
			TextBatch* batch;
			TextBatch::VBID vbid;

		public:
			Transformer2D transformer;

			GlyphText(TextBatch& text_batch);
			GlyphText(const GlyphText&);
			GlyphText(GlyphText&&) noexcept;
			GlyphText& operator=(const GlyphText&);
			GlyphText& operator=(GlyphText&&) noexcept;
			~GlyphText();

			void draw() const;

			void set_texture(const BindlessTextureRes& texture) const;
			void set_vertex_positions(const math::Rect2D& rect) const;
			void set_tex_coords(const math::Rect2D& rect) const;
			void set_foreground(const TextBatch::Foreground& foreground) const;
			void set_background(const TextBatch::Background& background) const;
			void set_modulation(const TextBatch::Modulation& modulation) const;

			BindlessTextureRes get_texture() const;
			math::Rect2D get_vertex_positions() const;
			math::Rect2D get_tex_coords() const;
			TextBatch::Foreground get_foreground() const;
			TextBatch::Background get_background() const;
			TextBatch::Modulation get_modulation() const;

			const TextBatch& get_batch() const { return *batch; }
			TextBatch& get_batch() { return *batch; }
			const Transform2D& get_local() const { return transformer.get_local(); }
			Transform2D& set_local() { return transformer.set_local(); }
		};
	}
}
