#pragma once

#include "physics/collision/debugging/CoreShapes.h"

namespace oly::debug
{
    constexpr float kInfiniteRayLength = 1'000'000.0f;

    extern DebugShapeGroup create_shape_group(
        const col2d::ContactResult::Contact& contact,
        glm::vec4 color = kStandardBlue,
        float arrow_width = 6.0f);
    
    extern void modify_shape_group(
        DebugOverlay& overlay,
        const col2d::ContactResult::Contact& contact,
        float arrow_width = 6.0f,
        size_t shape_index = 0);

    extern DebugShapeGroup create_shape_group(
        col2d::Ray ray,
        glm::vec4 color = kStandardBlue,
        float arrow_width = 6.0f);
    
    extern void modify_shape_group(
        DebugOverlay& overlay,
        col2d::Ray ray,
        float arrow_width = 6.0f,
        size_t shape_index = 0);

    extern DebugShapeGroup create_shape_group(
        const col2d::RaycastResult& result,
        glm::vec4 color = kStandardBlue,
        float impulse_length = 50.0f,
        float arrow_width = 6.0f);
    
    extern void modify_shape_group(
        DebugOverlay& overlay,
        const col2d::RaycastResult& result,
        float impulse_length = 50.0f,
        float arrow_width = 6.0f,
        size_t shape_index = 0);

    extern DebugShapeGroup create_shape_group(
        const col2d::RectCast& cast,
        glm::vec4 arrow_color = kStandardGreen,
        glm::vec4 bkg_color = kStandardBlue);
    
    extern void modify_shape_group(
        DebugOverlay& overlay,
        const col2d::RectCast& cast,
        float arrow_width = 6.0f,
        size_t shape_index = 0);

    extern DebugShapeGroup create_shape_group(
        const col2d::CircleCast& cast,
        glm::vec4 arrow_color = kStandardGreen,
        glm::vec4 bkg_color = kStandardBlue);
    
    extern void modify_shape_group(
        DebugOverlay& overlay,
        const col2d::CircleCast& cast,
        float arrow_width = 6.0f,
        size_t shape_index = 0);
}
