#pragma once

#include "physics/collision/methods/CollisionInfo.h"
#include "physics/collision/methods/SpecialCasting.h"
#include "physics/collision/debugging/DebugOverlay.h"

namespace oly::col2d::internal
{
	class ColliderObject;

	extern void load_luts();

	extern OverlapResult lut_point_hits(const ColliderObject&, glm::vec2);
	extern OverlapResult lut_ray_hits(const ColliderObject&, Ray);
	extern RaycastResult lut_raycast(const ColliderObject&, Ray);
	extern OverlapResult lut_overlaps(const ColliderObject&, const ColliderObject&);
	extern CollisionResult lut_collides(const ColliderObject&, const ColliderObject&);
	extern ContactResult lut_contacts(const ColliderObject&, const ColliderObject&);
	extern OverlapResult lut_circle_cast_hits(const ColliderObject&, const CircleCast&);
	extern OverlapResult lut_rect_cast_hits(const ColliderObject&, const RectCast&);

	extern math::Rect2D lut_flush(const ColliderObject&);
	extern bool lut_is_dirty(const ColliderObject&);
	extern debug::DebugOverlay lut_create_debug_overlay(debug::DebugOverlayLayer&, const ColliderObject&, glm::vec4, debug::DebugOverlay::PaintOptions = {});
	extern void lut_modify_debug_overlay(debug::DebugOverlay&, const ColliderObject&, size_t);

	extern const Transformer2D& lut_transformer(const ColliderObject&);
	extern Transformer2D& lut_transformer(ColliderObject&);

	extern Layer lut_layer(const ColliderObject&);
	extern Layer& lut_layer(ColliderObject&);
	extern Mask lut_mask(const ColliderObject&);
	extern Mask& lut_mask(ColliderObject&);
}
