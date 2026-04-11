#pragma once

#include "core/context/Public.h"

#include "core/base/Colors.h"
#include "core/base/SimpleMath.h"
#include "core/base/Random.h"

#include "core/util/Logger.h"
#include "core/util/Time.h"
#include "core/util/Timers.h"
#include "core/util/Loader.h"

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
#include "graphics/sprites/NineSlice.h"

#include "graphics/particles/ParticleSystem.h"
#include "graphics/particles/implementations/Samplers.h"
#include "graphics/particles/implementations/Domains.h"
#include "graphics/particles/implementations/Spawners.h"
#include "graphics/particles/implementations/AttributeOperations.h"

#include "graphics/Drawable.h"

#include "physics/collision/debugging/CommonShapes.h"

#include "physics/dynamics/Constants.h"
#include "physics/dynamics/bodies/StaticBody.h"
#include "physics/dynamics/bodies/LinearBody.h"
#include "physics/dynamics/bodies/KinematicBody.h"

namespace oly
{
	using context::run;

	inline platform::Window& context_window() { return context::get_platform().window(); }
	inline platform::Gamepad& context_gamepad() { return context::get_platform().gamepad(); }

	inline rendering::Camera2D& default_camera() { return *rendering::Camera2DRef(REF_DEFAULT); }

	// TODO v7 remove REPL -> go back to GUI, using gen tools for definitions
	// TODO v7 use versioning for all assets and definitions. create meta tools + conversion files to convert between versions
	// TODO v7 texture import metadata should have some additional size scale that affects any sprites using the texture
	// TODO v7 Coroutine system (including WaitForSeconds(), YieldFrame(), etc.)
	// TODO v7 Random class for random properties, similar to Unity
	// TODO v7 various utilities, including FuzzySearch
	// TODO v8 AI: Navigation, Blackboard Trees, etc.
	// TODO v8 Dialog/decision trees
	// TODO v8 Gameplay utilities such as TypewriterEffect, etc.
	// TODO v8 Lighting engine
	// TODO v8 UI widgets
	// TODO v9 thread safety + multi-threading.
	// TODO v10 network communication - online/local multiplayer.
}
