#pragma once

#include "core/context/Public.h"

#include "core/base/Color.h"
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

	// TODO v9 texture import metadata should have some additional size scale that affects any sprites using the texture
	// TODO v10 Random class for random properties, similar to Unity
	// TODO v10 Coroutine system (including WaitForSeconds(), YieldFrame(), etc.)
	// TODO v10 various utilities, including FuzzySearch
	// TODO v10 more of a ImGui style functional programming system for things like text rendering (among other things). For example, with text styling, no need to encode tags when building text from code - define a StyleBuilder that can push/pop styles. When parsing from an asset, it's simple to use. No need for compound font styles possibly.

	// TODO v11 lifetime/archetype design:
	// - no ECS
	// - behaviour is implemented in code, attributes are configured in assets
	// - define Archetype asset files: in GUI, it has a name (namespace + class name) and a list of data members (sprites, rigid bodies, other archetypes, primitives (bool/int/etc.), etc.).
	//   - You can open a member to set up all its attributes
	//   - You can open the draw view to set up the draw order of the members
	//   - You can open the tick view to set up the tick order of the members
	// - Editor will auto-generate Archetype files that implement draw and tick in the correct ordering of its members.
	// - Code-based behaviour scripts will extend a GameObject class that has virtual OnSpawn()/OnDespawn()/OnTick()/OnDraw()/etc. methods. The GameObject base class will auto-register those methods to the proper services.

	// TODO v10 RigidBody should use TransformerRef, not Transformer, to allow for driving physics on any transformer outside of the game object hierarchy.

	// TODO v10 Dialog/decision trees
	// TODO v10 Gameplay utilities such as TypewriterEffect, etc.
	// TODO v10 Rigid Body skeletons (joints, bones/springs, etc.)
	// TODO v12 AI: Navigation, Blackboard Trees, etc.
	// TODO v12 Lighting engine
	// TODO v12 UI widgets
	// TODO v13 thread safety + multi-threading.
	// TODO v14 network communication - online/local multiplayer.
	// TODO v10 change license to the same as used in Lexico
}
