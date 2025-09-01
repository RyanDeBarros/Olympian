#include "Text.h"

#include "core/context/rendering/Rendering.h"
#include "core/context/rendering/Fonts.h"
#include "registries/Loader.h"
#include "registries/graphics/text/FontAtlasRegistry.h"

namespace oly::context
{
	namespace internal
	{
		std::unique_ptr<rendering::TextBatch> text_batch;
	}

	void internal::init_text(const TOMLNode& node)
	{
		if (auto toml_text_batch = node["text_batch"])
		{
			int initial_glyphs = 0;
			reg::parse_int(toml_text_batch, "initial glyphs", initial_glyphs);
			int new_textures = 0;
			reg::parse_int(toml_text_batch, "new textures", new_textures);
			int new_text_colors = 0;
			reg::parse_int(toml_text_batch, "new text_colors", new_text_colors);
			int new_modulations = 0;
			reg::parse_int(toml_text_batch, "new modulations", new_modulations);

			rendering::TextBatch::Capacity capacity{ (GLuint)initial_glyphs, (GLuint)new_textures, (GLuint)new_text_colors, (GLuint)new_modulations };
			internal::text_batch = std::make_unique<rendering::TextBatch>(capacity);
		}
	}

	void internal::terminate_text()
	{
		internal::text_batch.reset();
	}

	rendering::TextBatch& text_batch()
	{
		return *internal::text_batch;
	}

	rendering::Paragraph paragraph(const std::string& font_atlas, const rendering::ParagraphFormat& format, utf::String&& text, unsigned int atlas_index)
	{
		return rendering::Paragraph(*internal::text_batch, font_atlas_registry().load_font_atlas(font_atlas, atlas_index), format, std::move(text));
	}

	void render_text()
	{
		internal::text_batch->render();
		internal::set_last_internal_batch_rendered(InternalBatch::TEXT);
	}
}
