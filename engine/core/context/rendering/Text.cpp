#include "Text.h"

#include "core/context/rendering/Fonts.h"
#include "registries/graphics/text/FontAtlasRegistry.h"

namespace oly::context
{
	rendering::Paragraph paragraph(const std::string& font_atlas, const rendering::ParagraphFormat& format, utf::String&& text, unsigned int atlas_index, rendering::SpriteBatch* batch)
	{
		return rendering::Paragraph(font_atlas_registry().load_font_atlas(font_atlas, atlas_index), format, std::move(text), batch);
	}
}
