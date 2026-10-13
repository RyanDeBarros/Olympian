#include "CoreShapes.h"

namespace oly::debug
{
    DebugShapeGroup create_shape_group(const col2d::Circle& c, glm::vec4 color)
    {
        rendering::StaticEllipse ellipse;
        ellipse.set_transform(c.compute_transform());
        auto dim = ellipse.get_dimension();
        dim.ry = dim.rx = c.radius;
        ellipse.set_dimension(dim);
        ellipse.set_color(color);
        return ellipse;
    }

    void modify_shape_group(DebugOverlay& overlay, const col2d::Circle& c, size_t shape_index)
    {
        if (auto obj = overlay[shape_index]->safe_get<rendering::StaticEllipse>())
        {
            obj->set_transform(c.compute_transform());
            auto dim = obj->get_dimension();
            dim.ry = dim.rx = c.radius;
            obj->set_dimension(dim);
            overlay.shapes_modified();
        }
        else
        {
            rendering::StaticEllipse ellipse = overlay.get_layer().create_ellipse();
            ellipse.set_transform(c.compute_transform());
            auto dim = ellipse.get_dimension();
            dim.ry = dim.rx = c.radius;
            ellipse.set_dimension(dim);
            overlay.set_shape_group(std::move(ellipse));
        }
    }

    DebugShapeGroup create_shape_group(const col2d::AABB& c, glm::vec4 color)
    {
        return internal::create_polygon_shape_group(c.points(), color);
    }

    void modify_shape_group(DebugOverlay& overlay, const col2d::AABB& c, size_t shape_index)
    {
        internal::modify_polygon_shape_group(overlay, c.points(), shape_index);
    }

    DebugShapeGroup create_shape_group(const col2d::OBB& c, glm::vec4 color)
    {
        return internal::create_polygon_shape_group(c.points(), color);
    }

    void modify_shape_group(DebugOverlay& overlay, const col2d::OBB& c, size_t shape_index)
    {
        internal::modify_polygon_shape_group(overlay, c.points(), shape_index);
    }

    DebugShapeGroup create_shape_group(const col2d::ConvexHull& c, glm::vec4 color)
    {
        return internal::create_polygon_shape_group(c.points(), color);
    }

    void modify_shape_group(DebugOverlay& overlay, const col2d::ConvexHull& c, size_t shape_index)
    {
        internal::modify_polygon_shape_group(overlay, c.points(), shape_index);
    }

    DebugShapeGroup create_shape_group(const col2d::Element& c, glm::vec4 color)
    {
        return c.variant().visit([color](const auto& e) { return create_shape_group(*e, color); });
    }

    void modify_shape_group(DebugOverlay& overlay, const col2d::Element& c, size_t shape_index)
    {
        c.variant().visit([&overlay, shape_index](const auto& e) { modify_shape_group(overlay, *e, shape_index); });
    }

    DebugShapeGroup create_shape_group(const col2d::Primitive& c, glm::vec4 color)
    {
        return create_shape_group(c.element, color);
    }

    void modify_shape_group(DebugOverlay& overlay, const col2d::Primitive& c, size_t shape_index)
    {
        modify_shape_group(overlay, c.element, shape_index);
    }

    DebugShapeGroup create_shape_group(const col2d::TPrimitive& c, glm::vec4 color)
    {
        return create_shape_group(c.get_baked(), color);
    }

    void modify_shape_group(DebugOverlay& overlay, const col2d::TPrimitive& c, size_t shape_index)
    {
        modify_shape_group(overlay, c.get_baked(), shape_index);
    }

    DebugShapeGroup create_shape_group(const col2d::Compound& c, glm::vec4 color)
    {
        return create_shape_group(c.elements, color);
    }

    void modify_shape_group(DebugOverlay& overlay, const col2d::Compound& c, size_t shape_index)
    {
        modify_shape_group(overlay, c.elements, shape_index);
    }

    DebugShapeGroup create_shape_group(const col2d::TCompound& c, glm::vec4 color)
    {
        return create_shape_group(c.get_baked(), color);
    }

    void modify_shape_group(DebugOverlay& overlay, const col2d::TCompound& c, size_t shape_index)
    {
        modify_shape_group(overlay, c.get_baked(), shape_index);
    }
}
