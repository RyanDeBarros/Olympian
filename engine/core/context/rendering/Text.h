#pragma once

#include "external/TOML.h"
#include "graphics/text/Paragraph.h"

namespace oly::context
{
	extern rendering::Paragraph paragraph(const std::string& font_atlas, const rendering::ParagraphFormat& format = {}, utf::String&& text = "",
		unsigned int atlas_index = 0, rendering::SpriteBatch* batch = rendering::CONTEXT_SPRITE_BATCH);
}
