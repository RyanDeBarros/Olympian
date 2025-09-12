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
			int initial_glyphs = (int)toml_text_batch["initial_glyphs"].value_or<int64_t>(0);
			int new_textures = (int)toml_text_batch["new_textures"].value_or<int64_t>(0);
			int new_modulations = (int)toml_text_batch["new_modulations"].value_or<int64_t>(0);

			rendering::TextBatch::Capacity capacity{ (GLuint)initial_glyphs, (GLuint)new_textures, (GLuint)new_modulations };
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

	rendering::Paragraph paragraph(const std::string& font_atlas, const rendering::ParagraphFormat& format, utf::String&& text, unsigned int atlas_index, rendering::SpriteBatch* batch)
	{
		return rendering::Paragraph(font_atlas_registry().load_font_atlas(font_atlas, atlas_index), format, std::move(text), batch);
	}

	void render_text()
	{
		internal::text_batch->render();
		internal::set_batch_rendering_tracker(InternalBatch::TEXT, false);
	}
}
