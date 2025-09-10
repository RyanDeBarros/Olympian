#pragma once

#include "core/context/Public.h"

#include "core/base/Colors.h"
#include "core/base/SimpleMath.h"

#include "core/util/Logger.h"
#include "core/util/Time.h"
#include "core/util/Timers.h"

#include "core/types/Approximate.h"
#include "core/types/SmartReference.h"

#include "graphics/backend/basic/Framebuffers.h"
#include "graphics/resources/Samplers.h"
#include "graphics/Stencil.h"

#include "graphics/primitives/Sprites.h"
#include "graphics/primitives/Polygons.h"
#include "graphics/primitives/Ellipses.h"
#include "graphics/extensions/TileMap.h"
#include "graphics/extensions/SpriteAtlas.h"
#include "graphics/extensions/SpriteNonant.h"
#include "graphics/text/Paragraph.h"

#include "physics/collision/debugging/CommonViews.h"

#include "physics/dynamics/Constants.h"
#include "physics/dynamics/bodies/StaticBody.h"
#include "physics/dynamics/bodies/LinearBody.h"
#include "physics/dynamics/bodies/KinematicBody.h"

namespace oly
{
	using rendering::Sprite;
	
	inline platform::Window& context_window() { return context::get_platform().window(); }
	inline platform::Gamepad& context_gamepad() { return context::get_platform().gamepad(); }

	// TODO v4 use likely/unlikely branching attributes.
	// TODO v4 Modular particle system, perlin noise generation.
	// TODO v5 lighting engine, ui widgets, thread safety + multi-threading.
	// TODO v5 network communication - online/local multiplayer.
}
