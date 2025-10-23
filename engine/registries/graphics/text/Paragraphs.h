#pragma once

#include "external/TOML.h"

#include "graphics/text/Paragraph.h"

namespace oly::reg
{
	extern rendering::Paragraph load_paragraph(TOMLNode node);
}
