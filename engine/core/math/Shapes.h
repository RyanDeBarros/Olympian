#pragma once

#include "external/GLM.h"
#include "external/TOML.h"

#include <array>

namespace oly::math
{
    struct Triangle2D
    {
        glm::vec2 root;
        glm::vec2 prev;
        glm::vec2 next;

        glm::vec2 dprev() const;
        glm::vec2 dnext() const;
        float signed_area() const;
        float area() const;
        float cross() const;
    };

    struct DirectedLine2D
    {
        glm::vec2 anchor;
        glm::vec2 dir;

        bool intersect(const DirectedLine2D& other, glm::vec2 pt) const;
    };

    struct Rect2D
    {
        float x1 = 0.f;
        float x2 = 0.f;
        float y1 = 0.f;
        float y2 = 0.f;

        bool operator==(const Rect2D&) const = default;

        float center_x() const;
        float center_y() const;
        glm::vec2 center() const;
        float width() const;
        float height() const;
        glm::vec2 size() const;

        std::array<glm::vec2, 4> uvs() const;

        bool valid() const;

        bool contains(glm::vec2 test) const;
        bool inside(const Rect2D& enclosing) const;
        bool strict_inside(const Rect2D& enclosing) const;
        bool overlaps(const Rect2D& other) const;
        glm::vec2 clamp(glm::vec2 pt) const;

        void include(glm::vec2 pt);
        void include_rect(const Rect2D& rect);

        Rect2D get_scaled(float sx, float sy) const;
        Rect2D get_scaled(glm::vec2 sc) const;
    };

    struct RotatedRect2D
    {
        glm::vec2 center = {};
        glm::vec2 size = {};
        float rotation = 0.f;

        std::array<glm::vec2, 4> points() const;
    };

    struct IRect2D
    {
        int x1 = 0;
        int x2 = 0;
        int y1 = 0;
        int y2 = 0;

        bool operator==(const IRect2D&) const = default;

        explicit operator Rect2D() const;

        static IRect2D from_size(glm::ivec2 size);
        static IRect2D round_out(Rect2D rect);

        static IRect2D load(TOMLNode node, bool validate = true);

        float center_x() const;
        float center_y() const;
        glm::vec2 center() const;
        bool contains(glm::ivec2 test) const;
        glm::ivec2 clamp(glm::ivec2 pt) const;
        int width() const;
        int height() const;
        glm::ivec2 size() const;
    };

    struct UVRect
    {
        float x1 = 0.f;
        float x2 = 1.f;
        float y1 = 0.f;
        float y2 = 1.f;

        bool operator==(const UVRect&) const = default;

        static UVRect from_grid(unsigned int row, unsigned int col, unsigned int rows, unsigned int cols);

        float interp_x(float alpha) const;
        float interp_y(float alpha) const;
    };

    struct Area2D
    {
        float x = 0.f;
        float y = 0.f;
        float w = 1.f;
        float h = 1.f;

        glm::vec2 center() const;
        glm::vec2 size() const;
    };

    struct IArea2D
    {
        int x = 0;
        int y = 0;
        int w = 1;
        int h = 1;
    };

    struct IArea3D
    {
        int x = 0;
        int y = 0;
        int z = 0;
        int w = 1;
        int h = 1;
        int d = 1;
    };

    struct Padding
    {
        float left = 0.f;
        float right = 0.f;
        float bottom = 0.f;
        float top = 0.f;

        static Padding load(TOMLNode node);
        static Padding uniform(float padding);
    };

    struct TopSidePadding
    {
        float left = 0.f;
        float right = 0.f;
        float top = 0.f;

        static TopSidePadding load(TOMLNode node);
        static TopSidePadding uniform(float padding);
    };
}

template<>
struct std::hash<oly::math::Rect2D>
{
    size_t operator()(oly::math::Rect2D r) const;
};

template<>
struct std::hash<oly::math::IRect2D>
{
    size_t operator()(oly::math::IRect2D r) const;
};

template<>
struct std::hash<oly::math::UVRect>
{
    size_t operator()(oly::math::UVRect r) const;
};
