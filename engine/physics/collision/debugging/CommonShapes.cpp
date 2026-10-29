#include "CommonShapes.h"

namespace oly::debug
{
    DebugShapeGroup create_shape_group(
        const col2d::ContactResult::Contact& contact,
        glm::vec4 color,
        float arrow_width)
    {
        rendering::StaticArrowExtension impulse;
        impulse.set_color(color);
        impulse.adjust_standard_head_for_width(arrow_width);
        impulse.set_start() = contact.position;
        impulse.set_end() = contact.position + contact.impulse;
        return impulse;
    }

    void modify_shape_group(
        DebugOverlay& overlay,
        const col2d::ContactResult::Contact& contact,
        float arrow_width,
        size_t shape_index)
    {
        if (auto obj = overlay[shape_index]->safe_get<rendering::StaticArrowExtension>())
        {
            obj->adjust_standard_head_for_width(arrow_width);
            obj->set_start() = contact.position;
            obj->set_end() = contact.position + contact.impulse;
            overlay.shapes_modified();
        }
        else
        {
            rendering::StaticArrowExtension impulse = overlay.get_layer().create_arrow();
            impulse.adjust_standard_head_for_width(arrow_width);
            impulse.set_start() = contact.position;
            impulse.set_end() = contact.position + contact.impulse;
            overlay.set_shape_group(std::move(impulse));
        }
    }

    DebugShapeGroup create_shape_group(col2d::Ray ray, glm::vec4 color, float arrow_width)
    {
        rendering::StaticArrowExtension arrow;
        arrow.set_color(color);
        arrow.adjust_standard_head_for_width(arrow_width);
        arrow.set_start() = ray.origin;
        float clip = ray.clip == 0.0f ? kInfiniteRayLength : ray.clip;
        arrow.set_end() = ray.origin + clip * (glm::vec2)ray.direction;
        return arrow;
    }

    void modify_shape_group(DebugOverlay& overlay, col2d::Ray ray, float arrow_width, size_t shape_index)
    {
        if (auto obj = overlay[shape_index]->safe_get<rendering::StaticArrowExtension>())
        {
            obj->adjust_standard_head_for_width(arrow_width);
            obj->set_start() = ray.origin;
            obj->set_end() = ray.clip == 0.0f
                ? (ray.origin + kInfiniteRayLength * (glm::vec2)ray.direction)
                : ray.origin + ray.clip * (glm::vec2)ray.direction;
            overlay.shapes_modified();
        }
        else
        {
            rendering::StaticArrowExtension arrow = overlay.get_layer().create_arrow();
            arrow.adjust_standard_head_for_width(arrow_width);
            arrow.set_start() = ray.origin;
            arrow.set_end() = ray.clip == 0.0f
                ? (ray.origin + kInfiniteRayLength * (glm::vec2)ray.direction)
                : ray.origin + ray.clip * (glm::vec2)ray.direction;
            overlay.set_shape_group(std::move(arrow));
        }
    }

    DebugShapeGroup create_shape_group(
        const col2d::RaycastResult& result,
        glm::vec4 color,
        float impulse_length,
        float arrow_width)
    {
        if (result.hit == col2d::RaycastResult::Hit::TrueHit)
        {
            col2d::Ray ray{ .origin = result.contact, .direction = result.normal, .clip = impulse_length };
            return create_shape_group(ray, color, arrow_width);
        }
        else
            return {};
    }

    void modify_shape_group(
        DebugOverlay& overlay,
        const col2d::RaycastResult& result,
        float impulse_length,
        float arrow_width,
        size_t shape_index)
    {
        if (result.hit == col2d::RaycastResult::Hit::TrueHit)
        {
            col2d::Ray ray{ .origin = result.contact, .direction = result.normal, .clip = impulse_length };
            modify_shape_group(overlay, ray, arrow_width, shape_index);
        }
        else
            overlay.clear_shape_group();
    }

    DebugShapeGroup create_shape_group(const col2d::RectCast& cast, glm::vec4 arrow_color, glm::vec4 bkg_color)
    {
        DebugShapeGroup overlay = create_shape_group(cast.finite_obb(kInfiniteRayLength), bkg_color);
        overlay.merge(create_shape_group(cast.ray, arrow_color));
        return overlay;
    }

    void modify_shape_group(DebugOverlay& overlay, const col2d::RectCast& cast, float arrow_width, size_t shape_index)
    {
        overlay.resize_shape_group(std::max(shape_index + 2, overlay.shape_count()));
        modify_shape_group(overlay, cast.finite_obb(kInfiniteRayLength), shape_index);
        modify_shape_group(overlay, cast.ray, arrow_width, shape_index + 1);
    }

    DebugShapeGroup create_shape_group(const col2d::CircleCast& cast, glm::vec4 arrow_color, glm::vec4 bkg_color)
    {
        DebugShapeGroup overlay = create_shape_group(cast.finite_capsule(kInfiniteRayLength).compound(), bkg_color);
        overlay.merge(create_shape_group(cast.ray, arrow_color));
        return overlay;
    }

    void modify_shape_group(
        DebugOverlay& overlay,
        const col2d::CircleCast& cast,
        float arrow_width,
        size_t shape_index)
    {
        overlay.resize_shape_group(std::max(shape_index + 4, overlay.shape_count()));
        modify_shape_group(overlay, cast.finite_capsule(kInfiniteRayLength).mid_obb(), shape_index);
        modify_shape_group(overlay, cast.finite_capsule(kInfiniteRayLength).lower_circle(), shape_index + 1);
        modify_shape_group(overlay, cast.finite_capsule(kInfiniteRayLength).upper_circle(), shape_index + 2);
        modify_shape_group(overlay, cast.ray, arrow_width, shape_index + 3);
    }
}
