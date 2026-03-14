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
#include "graphics/sprites/SpriteNonant.h"

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

	/*
	TODO v7 redesign editor again - remove current editor folder, and do the following:

	Editor should essentially be embedded in a text editor. Create REPL program that can use files
	for large input/output (for example, open asset file in this buffer file). Any file operations
	can be managed by the user instead of a content browser. The only thing the editor can take care
	of is generating import files or assets, and handling deletion/moving/copying of files with import
	files.

	Eventually, can create standardized format for asset serialization/deserialization and prebuilder:
	Define format files that associate attributes with values (including data types, defaults,
	subtypes, etc.) for classes. In loader methods, call generated header files. The prebuilder should
	be an engine-level prebuilder, not project-level prebuilder. The editor can also use these format
	files in order to generate and parse asset files.
	*/

	// TODO v7 Coroutine system (including WaitForSeconds(), YieldFrame(), etc.)
	// TODO v8 Lighting engine
	// TODO v8 UI widgets
	// TODO v9 thread safety + multi-threading.
	// TODO v10 network communication - online/local multiplayer.
}
