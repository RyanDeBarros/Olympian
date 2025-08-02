#pragma once

// TODO v3 core/context/Public.h
#include "core/context/Context.h"
#include "core/context/Rendering.h"
#include "core/context/Platform.h"
#include "core/context/Registries.h"

#include "core/base/Colors.h"
#include "core/base/SimpleMath.h"
#include "core/util/Logger.h"
#include "core/util/Time.h"
#include "core/util/Timer.h"
#include "core/types/Approximate.h"

#include "graphics/resources/Samplers.h"
#include "graphics/Stencil.h"
#include "graphics/backend/basic/Framebuffers.h"

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
	using rendering::Sprite;
}
