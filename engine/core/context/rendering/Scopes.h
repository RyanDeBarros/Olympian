#pragma once

#include "graphics/Camera.h"

#include "graphics/backend/basic/Framebuffers.h"

namespace oly::context
{
    class ScopedViewportChange
    {
        const rendering::Camera2D& camera_;
        glm::vec4 original_clear_color_;
        bool original_blend_enabled_;

    public:
        ScopedViewportChange(
            const rendering::Camera2D& camera,
            glm::vec4 clear_color,
            bool blend_enabled,
            glm::ivec2 viewport_size,
            glm::ivec2 viewport_pos = {});

        ScopedViewportChange(const ScopedViewportChange&) = delete;
        ScopedViewportChange(ScopedViewportChange&&) = delete;
        ~ScopedViewportChange();
    };
}
