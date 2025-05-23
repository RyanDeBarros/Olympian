#pragma once

#include "core/base/Context.h"
#include "core/base/Colors.h"
#include "core/base/SimpleMath.h"
#include "core/util/Logger.h"
#include "core/util/Time.h"

#include "graphics/resources/Samplers.h"
#include "graphics/Stencil.h"

#include "registries/Loader.h"
#include "registries/graphics/primitives/Sprites.h"
#include "registries/graphics/primitives/Polygons.h"
#include "registries/graphics/primitives/Ellipses.h"
#include "registries/graphics/extensions/TileMaps.h"
#include "registries/graphics/extensions/SpriteAtlases.h"
#include "registries/graphics/extensions/SpriteNonants.h"
#include "registries/graphics/text/Paragraphs.h"

namespace oly
{
	using context::get_platform;
	using context::sprite;

	using rendering::Sprite;
}
