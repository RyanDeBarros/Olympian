#pragma once

#include "core/context/Public.h"

#include "core/base/Approximate.h"
#include "core/base/Color.h"
#include "core/base/Random.h"
#include "core/base/SimpleMath.h"
#include "core/base/SmartReference.h"

#include "core/util/Logger.h"
#include "core/util/Time.h"
#include "core/util/Timers.h"
#include "core/util/Loader.h"

#include "graphics/Drawable.h"
#include "graphics/Stencil.h"

#include "graphics/backend/basic/Framebuffers.h"

#include "graphics/particles/ParticleSystem.h"
#include "graphics/particles/implementations/Samplers.h"
#include "graphics/particles/implementations/Domains.h"
#include "graphics/particles/implementations/Spawners.h"
#include "graphics/particles/implementations/AttributeOperations.h"

#include "graphics/resources/Samplers.h"
#include "graphics/resources/Textures.h"

#include "graphics/sprites/Sprite.h"
#include "graphics/sprites/TileMap.h"
#include "graphics/sprites/SpriteAtlas.h"
#include "graphics/sprites/NineSlice.h"

#include "graphics/text/Paragraph.h"

#include "physics/collision/debugging/CommonShapes.h"

#include "physics/dynamics/Constants.h"

#include "physics/dynamics/bodies/StaticBody.h"
#include "physics/dynamics/bodies/LinearBody.h"
#include "physics/dynamics/bodies/KinematicBody.h"

namespace oly
{
    using context::Context;

    extern platform::Window& context_window();
    extern platform::Gamepad& context_gamepad();
    extern rendering::Camera2D& default_camera();
}
