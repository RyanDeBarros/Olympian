#include "Shapes.h"

#include "core/base/Approximate.h"
#include "core/base/Transforms.h"

#include "core/math/Geometry.h"

#include "core/util/Parser.h"
#include "core/util/Logger.h"

#include "definitions/Keys.h"

#include <imp/hash.hpp>

namespace oly::math
{
    glm::vec2 Triangle2D::dprev() const
    {
        return root - prev;
    }

    glm::vec2 Triangle2D::dnext() const
    {
        return next - root;
    }

    float Triangle2D::signed_area() const
    {
        return 0.5f * (root.x * (prev.y - next.y) + prev.x * (next.y - root.y) + next.x * (root.y - prev.y));
    }

    float Triangle2D::area() const
    {
        return std::abs(signed_area());
    }

    float Triangle2D::cross() const
    {
        return math::cross(root - prev, next - root);
    }

    bool DirectedLine2D::intersect(const DirectedLine2D& other, glm::vec2 pt) const
    {
        glm::mat2 D = { dir, -other.dir };
        if (near_zero(glm::determinant(D)))
            return false;

        glm::mat2 Dinv = glm::inverse(D);
        glm::vec2 P = other.anchor - anchor;
        glm::vec2 T = Dinv * P;
        pt = anchor + T.x * dir;

        return true;
    }

    float Rect2D::center_x() const
    {
        return 0.5f * (x1 + x2);
    }
    
    float Rect2D::center_y() const
    {
        return 0.5f * (y1 + y2);
    }
    
    glm::vec2 Rect2D::center() const
    {
        return 0.5f * glm::vec2{ x1 + x2, y1 + y2 };
    }
    
    float Rect2D::width() const
    {
        return x2 - x1;
    }
    
    float Rect2D::height() const
    {
        return y2 - y1;
    }
    
    glm::vec2 Rect2D::size() const
    {
        return { x2 - x1, y2 - y1 };
    }

    std::array<glm::vec2, 4> Rect2D::uvs() const
    {
        return {
            glm::vec2{ x1, y1 },
            glm::vec2{ x2, y1 },
            glm::vec2{ x2, y2 },
            glm::vec2{ x1, y2 }
        };
    }

    bool Rect2D::valid() const
    {
        return x1 <= x2 && y1 <= y2;
    }

    bool Rect2D::contains(glm::vec2 test) const
    {
        return test.x >= x1 && test.x <= x2 && test.y >= y1 && test.y <= y2;
    }
    
    bool Rect2D::inside(const Rect2D& enclosing) const
    {
        return x1 >= enclosing.x1 && x2 <= enclosing.x2 && y1 >= enclosing.y1 && y2 <= enclosing.y2;
    }
    
    bool Rect2D::strict_inside(const Rect2D& enclosing) const
    {
        return x1 > enclosing.x1 && x2 < enclosing.x2 && y1 > enclosing.y1 && y2 < enclosing.y2;
    }
    
    bool Rect2D::overlaps(const Rect2D& other) const
    {
        return x1 <= other.x2 && other.x1 <= x2 && y1 <= other.y2 && other.y1 <= y2;
    }
    
    glm::vec2 Rect2D::clamp(glm::vec2 pt) const
    {
        return { glm::clamp(pt.x, x1, x2), glm::clamp(pt.y, y1, y2) };
    }

    void Rect2D::include(glm::vec2 pt)
    {
        x1 = glm::min(x1, pt.x); x2 = glm::max(x2, pt.x); y1 = glm::min(y1, pt.y); y2 = glm::max(y2, pt.y);
    }
    
    void Rect2D::include_rect(const Rect2D& rect)
    {
        if (rect.valid())
        {
            include({ rect.x1, rect.y1 });
            include({ rect.x2, rect.y2 });
        }
    }

    Rect2D Rect2D::get_scaled(float sx, float sy) const
    {
        return { .x1 = x1 * sx, .x2 = x2 * sx, .y1 = y1 * sy, .y2 = y2 * sy };
    }
    
    Rect2D Rect2D::get_scaled(glm::vec2 sc) const
    {
        return get_scaled(sc.x, sc.y);
    }

    std::array<glm::vec2, 4> RotatedRect2D::points() const
    {
        std::array<glm::vec2, 4> points{
            glm::vec2{ -0.5f * size.x, -0.5f * size.y },
            glm::vec2{  0.5f * size.x, -0.5f * size.y },
            glm::vec2{  0.5f * size.x,  0.5f * size.y },
            glm::vec2{ -0.5f * size.x,  0.5f * size.y }
        };

        glm::mat2 rot = rotation_matrix_2x2(rotation);
        for (glm::vec2& point : points)
            point = center + rot * point;
        
        return points;
    }

    IRect2D::operator Rect2D() const
    {
        return Rect2D{
            .x1 = static_cast<float>(x1),
            .x2 = static_cast<float>(x2),
            .y1 = static_cast<float>(y1),
            .y2 = static_cast<float>(y2)
        };
    }

    IRect2D IRect2D::from_size(glm::ivec2 size)
    {
        return { .x1 = 0, .x2 = size.x, .y1 = 0, .y2 = size.y };
    }
    
    IRect2D IRect2D::round_out(Rect2D rect)
    {
        return {
            .x1 = static_cast<int>(floor(rect.x1)),
            .x2 = static_cast<int>(ceil(rect.x2)),
            .y1 = static_cast<int>(floor(rect.y1)),
            .y2 = static_cast<int>(ceil(rect.y2))
        };
    }

    IRect2D IRect2D::load(TOMLNode node, bool validate)
    {
        if (!node)
            return {};

        assets::Parser parser(node);

        IRect2D rect;
        parser.optional(detail::Key::X1)(rect.x1);
        parser.optional(detail::Key::X2)(rect.x2);
        parser.optional(detail::Key::Y1)(rect.y1);
        parser.optional(detail::Key::Y2)(rect.y2);

        if (validate)
        {
            if (rect.x2 <= rect.x1 || rect.y2 <= rect.y1)
            {
                _OLY_ENGINE_LOG_ERROR("CONTEXT") << "cannot parse IRect2D "
                    << "(x1=" << rect.x1
                    << ";x2=" << rect.x2
                    << ";y1=" << rect.y1
                    << ";y2=" << rect.y2
                    << ") - invalid bounds" << kLog.endl;

                throw Error(ErrorCode::LoadAsset);
            }
        }

        return rect;
    }

    float IRect2D::center_x() const
    {
        return 0.5f * (x1 + x2);
    }

    float IRect2D::center_y() const
    {
        return 0.5f * (y1 + y2);
    }

    glm::vec2 IRect2D::center() const
    {
        return 0.5f * glm::vec2{ x1 + x2, y1 + y2 };
    }

    bool IRect2D::contains(glm::ivec2 test) const
    {
        return test.x >= x1 && test.x <= x2 && test.y >= y1 && test.y <= y2;
    }

    glm::ivec2 IRect2D::clamp(glm::ivec2 pt) const
    {
        return { glm::clamp(pt.x, x1, x2), glm::clamp(pt.y, y1, y2) };
    }

    int IRect2D::width() const
    {
        return x2 - x1;
    }

    int IRect2D::height() const
    {
        return y2 - y1;
    }

    glm::ivec2 IRect2D::size() const
    {
        return { x2 - x1, y2 - y1 };
    }

    UVRect UVRect::from_grid(unsigned int row, unsigned int col, unsigned int rows, unsigned int cols)
    {
        return {
            .x1 = col / static_cast<float>(cols),
            .x2 = (col + 1) / static_cast<float>(cols),
            .y1 = row / static_cast<float>(rows),
            .y2 = (row + 1) / static_cast<float>(rows)
        };
    }

    float UVRect::interp_x(float alpha) const
    {
        return (x2 - x1) * alpha + x1;
    }
    
    float UVRect::interp_y(float alpha) const
    {
        return (y2 - y1) * alpha + y1;
    }

    glm::vec2 Area2D::center() const
    {
        return glm::vec2{ x + 0.5f * w, y + 0.5f * h };
    }

    glm::vec2 Area2D::size() const
    {
        return { w, h };
    }

    Padding Padding::load(TOMLNode node)
    {
        if (!node)
            return {};

        assets::Parser parser(node);

        Padding padding;

        if (auto uniform = parser.optional<double>(detail::Key::Uniform)())
            padding = Padding::uniform(*uniform);

        parser.optional(detail::Key::Left)(padding.left);
        parser.optional(detail::Key::Right)(padding.right);
        parser.optional(detail::Key::Top)(padding.top);
        parser.optional(detail::Key::Bottom)(padding.bottom);

        return padding;
    }

    Padding Padding::uniform(float padding)
    {
        return {
            .left = padding,
            .right = padding,
            .bottom = padding,
            .top = padding
        };
    }

    TopSidePadding TopSidePadding::load(TOMLNode node)
    {
        if (!node)
            return {};

        assets::Parser parser(node);

        TopSidePadding padding;

        if (auto uniform = parser.optional<double>(detail::Key::Uniform)())
            padding = TopSidePadding::uniform(*uniform);

        parser.optional(detail::Key::Left)(padding.left);
        parser.optional(detail::Key::Right)(padding.right);
        parser.optional(detail::Key::Top)(padding.top);

        return padding;
    }

    TopSidePadding TopSidePadding::uniform(float padding)
    {
        return {
            .left = padding,
            .right = padding,
            .top = padding
        };
    }
}


size_t std::hash<oly::math::Rect2D>::operator()(oly::math::Rect2D r) const
{
    return imp::hasher()
        .with(r.x1)
        .with(r.x2)
        .with(r.y1)
        .with(r.y2);
}

size_t std::hash<oly::math::IRect2D>::operator()(oly::math::IRect2D r) const
{
    return imp::hasher()
        .with(r.x1)
        .with(r.x2)
        .with(r.y1)
        .with(r.y2);
}

size_t std::hash<oly::math::UVRect>::operator()(oly::math::UVRect r) const
{
    return imp::hasher()
        .with(r.x1)
        .with(r.x2)
        .with(r.y1)
        .with(r.y2);
}
