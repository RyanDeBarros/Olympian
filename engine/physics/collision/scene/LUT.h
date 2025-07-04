#pragma once

#include "physics/collision/methods/CollisionInfo.h"
#include "physics/collision/methods/SpecialCasting.h"
#include "physics/collision/debugging/CollisionView.h"
#include "core/math/Shapes.h"

namespace oly::col2d::internal
{
	class ColliderObject;

	extern void load_luts();

	extern OverlapResult lut_point_hits(const ColliderObject&, glm::vec2);
	extern OverlapResult lut_ray_hits(const ColliderObject&, const Ray&);
	extern RaycastResult lut_raycast(const ColliderObject&, const Ray&);
	extern OverlapResult lut_overlaps(const ColliderObject&, const ColliderObject&);
	extern CollisionResult lut_collides(const ColliderObject&, const ColliderObject&);
	extern ContactResult lut_contacts(const ColliderObject&, const ColliderObject&);
	extern OverlapResult lut_circle_cast_hits(const ColliderObject&, const CircleCast&);
	extern OverlapResult lut_rect_cast_hits(const ColliderObject&, const RectCast&);

	extern math::Rect2D lut_flush(const ColliderObject&);
	extern bool lut_is_dirty(const ColliderObject&);
	extern debug::CollisionView lut_collision_view(const ColliderObject&, glm::vec4);
	extern void lut_update_view(debug::CollisionView&, const ColliderObject&, glm::vec4);

	extern const Transformer2D& lut_transformer(const ColliderObject&);
	extern Transformer2D& lut_transformer(ColliderObject&);
}
