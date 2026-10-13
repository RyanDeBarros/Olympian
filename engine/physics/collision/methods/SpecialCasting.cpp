#include "SpecialCasting.h"

namespace oly::col2d
{
    OBB RectCast::finite_obb(float infinite_clip) const
    {
        float clip = ray.clip == 0.0f ? infinite_clip : ray.clip;
        return OBB{
            .center = ray.origin + 0.5f * clip * (glm::vec2)ray.direction,
            .width = depth + clip,
            .height = width,
            .rotation = ray.direction.rotation()
        };
    }

    Capsule CircleCast::finite_capsule(float infinite_clip) const
    {
        float clip = ray.clip == 0.0f ? infinite_clip : ray.clip;
        return Capsule{
            .center = ray.origin + 0.5f * clip * (glm::vec2)ray.direction,
            .obb_width = 2.0f * radius,
            .obb_height = clip,
            .rotation = (-ray.direction.get_quarter_turn()).rotation()
        };
    }
}
