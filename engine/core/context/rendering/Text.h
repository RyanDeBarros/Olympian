#pragma once

#include "external/TOML.h"
#include "graphics/text/Paragraph.h"

namespace oly::context
{
	namespace internal
	{
		extern void init_text(const TOMLNode&);
		extern void terminate_text();
	}

	extern rendering::TextBatch& text_batch();
	extern rendering::Paragraph paragraph(const std::string& font_atlas, const rendering::ParagraphFormat& format = {}, utf::String&& text = "",
		unsigned int atlas_index = 0, rendering::TextBatch* batch = rendering::CONTEXT_TEXT_BATCH);
	extern void render_text();
}
