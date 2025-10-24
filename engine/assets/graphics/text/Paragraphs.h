#pragma once

#include "external/TOML.h"

#include "graphics/text/Paragraph.h"

namespace oly::assets
{
	extern rendering::Paragraph load_paragraph(TOMLNode node, const char* source = nullptr);
}
