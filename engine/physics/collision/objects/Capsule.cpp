#include "Capsule.h"

namespace oly::col2d
{
    glm::mat2 Capsule::get_rotation_matrix() const
    {
        float cos = glm::cos(rotation);
        float sin = glm::sin(rotation);
        return { { cos, sin }, { -sin, cos } };
    }

    OBB Capsule::mid_obb() const
    {
        return {
            .center = center,
            .width = obb_width,
            .height = obb_height,
            .rotation = rotation
        };
    }
    
    Circle Capsule::lower_circle() const
    {
        return Circle(center + get_rotation_matrix() * glm::vec2{ 0.0f, -0.5f * obb_height }, 0.5f * obb_width);
    }
    
    Circle Capsule::upper_circle() const
    {
        return Circle(center + get_rotation_matrix() * glm::vec2{ 0.0f,  0.5f * obb_height }, 0.5f * obb_width);
    }

    UnitVector2D Capsule::horizontal() const
    {
        return UnitVector2D(rotation);
    }

    UnitVector2D Capsule::vertical() const
    {
        return UnitVector2D(rotation + glm::half_pi<float>());
    }

    Compound Capsule::compound() const
    {
        return { .elements = { mid_obb(), lower_circle(), upper_circle() } };
    }
    
    TCompound Capsule::tcompound() const
    {
        return TCompound({ mid_obb(), lower_circle(), upper_circle() });
    }
}
