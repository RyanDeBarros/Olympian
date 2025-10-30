#pragma once

#include "core/context/Public.h"

#include "core/base/Colors.h"
#include "core/base/SimpleMath.h"
#include "core/base/Random.h"

#include "core/util/Logger.h"
#include "core/util/Time.h"
#include "core/util/Timers.h"

#include "core/types/Approximate.h"
#include "core/types/SmartReference.h"

#include "graphics/backend/basic/Framebuffers.h"
#include "graphics/resources/Samplers.h"
#include "graphics/resources/Textures.h"
#include "graphics/Stencil.h"

#include "graphics/sprites/Sprite.h"
#include "graphics/text/Paragraph.h"
#include "graphics/sprites/TileMap.h"
#include "graphics/sprites/SpriteAtlas.h"
#include "graphics/sprites/SpriteNonant.h"

#include "graphics/Drawable.h"

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

	inline rendering::Camera2D& default_camera() { return *rendering::Camera2DRef(REF_DEFAULT); }

	// TODO v6 Modular particle system, perlin noise generation.
	// TODO v6 Lighting engine
	// TODO v7 UI widgets
	// TODO v8 thread safety + multi-threading.
	// TODO v9 network communication - online/local multiplayer.
}
