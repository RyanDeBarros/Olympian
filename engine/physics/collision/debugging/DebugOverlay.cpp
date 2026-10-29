#include "DebugOverlay.h"

namespace oly::debug
{
    DebugShape::DebugShape(Variant&& shape)
        : shape(std::move(shape))
    {
    }

    DebugShape::DebugShape(Variant&& shape, DebugOverlayLayer& layer_)
        : shape(std::move(shape))
    {
        align_layer_batch(layer_);
    }

    DebugShape::Variant& DebugShape::operator*()
    {
        return shape;
    }

    const DebugShape::Variant& DebugShape::operator*() const
    {
        return shape;
    }

    DebugShape::Variant* DebugShape::operator->()
    {
        return &shape;
    }

    const DebugShape::Variant* DebugShape::operator->() const
    {
        return &shape;
    }

    void DebugShape::draw() const
    {
        shape.visit([](const auto& s) { s.draw(); });
    }

    math::Rect2D DebugShape::bounds() const
    {
        return shape.visit(
            [](const rendering::StaticEllipse& s) {
            return s.bounds();
        },
            [](const rendering::StaticPolygon& s) {
            const auto& points = s.get_points();
            return col2d::AABB::wrap(points.data(), points.size()).rect();
        },
            [](const rendering::StaticArrowExtension& s) {
            const auto points = s.get_all_points();
            return col2d::AABB::wrap(points.data(), points.size()).rect();
        }
        );
    }

    math::RotatedRect2D DebugShape::rotated_bounds() const
    {
        return shape.visit(
            [](const rendering::StaticEllipse& s) {
            return s.rotated_bounds();
        },
            [](const rendering::StaticPolygon& s) {
            const auto& points = s.get_points();
            return col2d::OBB::fast_wrap(points.data(), points.size()).rect();
        },
            [](const rendering::StaticArrowExtension& s) {
            const auto points = s.get_all_points();
            return col2d::OBB::fast_wrap(points.data(), points.size()).rect();
        }
        );
    }

    void DebugShape::align_layer_batch(DebugOverlayLayer& layer_)
    {
        shape.visit(
            [&layer_](rendering::StaticEllipse& v) { v.set_batch(layer_.painter_.get_ellipse_batch()); },
            [&layer_](rendering::StaticPolygon& v) { v.set_batch(layer_.painter_.get_polygon_batch()); },
            [&layer_](rendering::StaticArrowExtension& v) { v.set_batch(layer_.painter_.get_polygon_batch()); }
        );
    }

    DebugShapeGroup::DebugShapeGroup(DebugShape&& shape)
    {
        shapes.push_back(std::move(shape));
    }

    DebugShapeGroup::DebugShapeGroup(rendering::StaticEllipse&& shape)
    {
        shapes.push_back(DebugShape(std::move(shape)));
    }

    DebugShapeGroup::DebugShapeGroup(rendering::StaticPolygon&& shape)
    {
        shapes.push_back(DebugShape(std::move(shape)));
    }

    DebugShapeGroup::DebugShapeGroup(rendering::StaticArrowExtension&& v)
    {
        shapes.push_back(DebugShape(std::move(v)));
    }

    void DebugShapeGroup::align_layer_batch(DebugOverlayLayer& layer_)
    {
        for (DebugShape& shape : shapes)
            shape.align_layer_batch(layer_);
    }

    void DebugShapeGroup::merge(DebugShapeGroup&& o)
    {
        if (o.shapes.empty())
            return;

        if (shapes.empty())
        {
            shapes = std::move(o.shapes);
            return;
        }

        shapes.insert(
            shapes.end(),
            std::make_move_iterator(o.shapes.begin()),
            std::make_move_iterator(o.shapes.end()));
    }

    DebugOverlay::DebugOverlay(DebugOverlayLayer& layer, PaintOptions paint_options)
        : layer_(&layer)
        , sprite_(rendering::kUnbatched)
        , paint_options(paint_options)
    {
        layer.assign(this);
    }

    DebugOverlay::DebugOverlay(DebugOverlayLayer& layer, rendering::StaticEllipse&& shape, PaintOptions paint_options)
        : layer_(&layer)
        , debug_group_(DebugShape(std::move(shape), layer))
        , sprite_(rendering::kUnbatched)
        , paint_options(paint_options)
    {
        layer.assign(this);
    }

    DebugOverlay::DebugOverlay(DebugOverlayLayer& layer, rendering::StaticPolygon&& shape, PaintOptions paint_options)
        : layer_(&layer)
        , debug_group_(DebugShape(std::move(shape), layer))
        , sprite_(rendering::kUnbatched)
        , paint_options(paint_options)
    {
        layer.assign(this);
    }

    DebugOverlay::DebugOverlay(
        DebugOverlayLayer& layer,
        rendering::StaticArrowExtension&& shape,
        PaintOptions paint_options)
        : layer_(&layer)
        , debug_group_(DebugShape(std::move(shape), layer))
        , sprite_(rendering::kUnbatched)
        , paint_options(paint_options)
    {
        layer.assign(this);
    }

    DebugOverlay::DebugOverlay(DebugOverlayLayer& layer, DebugShapeGroup&& group, PaintOptions paint_options)
        : layer_(&layer)
        , debug_group_(std::move(group))
        , sprite_(rendering::kUnbatched)
        , paint_options(paint_options)
    {
        debug_group_.align_layer_batch(layer);
        layer.assign(this);
    }

    DebugOverlay::DebugOverlay(const DebugOverlay& o)
        : layer_(o.layer_)
        , debug_group_(o.debug_group_)
        , sprite_(o.sprite_)
        , dirty_(o.dirty_)
    {
        if (layer_)
            layer_->assign(this);
    }

    DebugOverlay::DebugOverlay(DebugOverlay&& o) noexcept
        : layer_(o.layer_)
        , debug_group_(std::move(o.debug_group_))
        , sprite_(std::move(o.sprite_))
        , dirty_(o.dirty_)
    {
        if (layer_)
        {
            layer_->collision_views_.erase(&o);
            layer_->collision_views_.insert(this);
        }
    }

    DebugOverlay::~DebugOverlay()
    {
        if (layer_)
            layer_->unassign(this);
    }

    DebugOverlay& DebugOverlay::operator=(const DebugOverlay& o)
    {
        if (this != &o)
        {
            debug_group_ = o.debug_group_;
            shapes_modified();
        }

        return *this;
    }

    DebugOverlay& DebugOverlay::operator=(DebugOverlay&& o) noexcept
    {
        if (this != &o)
        {
            debug_group_ = std::move(o.debug_group_);
            shapes_modified();
        }

        return *this;
    }

    const DebugOverlayLayer& DebugOverlay::get_layer() const
    {
        if (layer_)
            return *layer_;
        else
            throw Error(ErrorCode::NullPointer);
    }

    DebugOverlayLayer& DebugOverlay::get_layer()
    {
        if (layer_)
            return *layer_;
        else
            throw Error(ErrorCode::NullPointer);
    }

    void DebugOverlay::set_layer(DebugOverlayLayer& layer_)
    {
        if (this->layer_)
        {
            if (this->layer_ == &layer_)
                return;

            this->layer_->unassign(this);
        }

        this->layer_ = &layer_;
        this->layer_->assign(this);

        debug_group_.align_layer_batch(layer_);
    }

    glm::vec2 DebugOverlay::PaintOptions::compress(math::Rect2D& bounds) const
    {
        glm::vec2 scale = glm::vec2(1.0f);

        if (quality < 1.0f)
        {
            if (bounds.width() > preferred_min_size.x)
            {
                float w = std::lerp(preferred_min_size.x, bounds.width(), quality.get());
                scale.x = bounds.width() / w;
                float offset = 0.5f * (bounds.width() - w);
                bounds.x1 += offset;
                bounds.x2 -= offset;
            }

            if (bounds.height() > preferred_min_size.y)
            {
                float h = std::lerp(preferred_min_size.y, bounds.height(), quality.get());
                scale.y = bounds.height() / h;
                float offset = 0.5f * (bounds.height() - h);
                bounds.y1 += offset;
                bounds.y2 -= offset;
            }
        }

        return scale;
    }

    void DebugOverlay::clear_shape_group()
    {
        debug_group_.shapes.clear();
        shapes_modified();
    }

    void DebugOverlay::set_shape_group(DebugShapeGroup&& group)
    {
        debug_group_ = std::move(group);
        shapes_modified();
    }

    void DebugOverlay::set_shape_group(rendering::StaticEllipse&& shape)
    {
        if (layer_)
            set_shape_group(DebugShapeGroup(DebugShape(std::move(shape), *layer_)));
    }

    void DebugOverlay::set_shape_group(rendering::StaticPolygon&& shape)
    {
        if (layer_)
            set_shape_group(DebugShapeGroup(DebugShape(std::move(shape), *layer_)));
    }

    void DebugOverlay::set_shape_group(rendering::StaticArrowExtension&& shape)
    {
        if (layer_)
            set_shape_group(DebugShapeGroup(DebugShape(std::move(shape), *layer_)));
    }

    size_t DebugOverlay::shape_count() const
    {
        return debug_group_.shapes.size();
    }

    void DebugOverlay::resize_shape_group(size_t size)
    {
        debug_group_.shapes.resize(size, layer_->default_debug_shape());
        shapes_modified();
    }

    const DebugShape& DebugOverlay::operator[](size_t i) const
    {
        return debug_group_.shapes[i];
    }

    DebugShape& DebugOverlay::operator[](size_t i)
    {
        shapes_modified();
        return debug_group_.shapes[i];
    }

    void DebugOverlay::shapes_modified() const
    {
        dirty_ = true;
    }

    static bool update_view_color(DebugShape& obj, glm::vec4 color)
    {
        return obj->visit(
            [color](rendering::StaticEllipse& obj) {
            if (!obj.get_color().is_uniform() || obj.get_color().fill_inner != color)
            {
                obj.set_color(color);
                return true;
            }
            else
                return false;
        },
            [color](rendering::StaticPolygon& obj) {
            const auto& colors = obj.get_colors();
            if (!(colors.size() == 1 && colors[0] == color))
            {
                obj.set_colors() = { color };
                return true;
            }
            else
                return false;
        },
            [color](rendering::StaticArrowExtension& obj) {
            if (obj.get_start_color() != obj.get_end_color() || obj.set_start_color() != color)
            {
                obj.set_color(color);
                return true;
            }
            else
                return false;
        }
        );
    }

    void DebugOverlay::update_color(glm::vec4 color)
    {
        bool updated = false;
        for (DebugShape& shape : debug_group_.shapes)
            updated |= update_view_color(shape, color);

        if (updated)
            shapes_modified();
    }

    void DebugOverlay::update_color(glm::vec4 color, size_t shape_index)
    {
        if (update_view_color(debug_group_.shapes[shape_index], color))
            shapes_modified();
    }

    void DebugOverlay::draw_sprite() const
    {
        if (dirty_)
            repaint();

        sprite_.draw();
    }

    void DebugOverlay::repaint() const
    {
        auto sprite_batch_ = sprite_.get_batch();
        if (!sprite_batch_)
        {
            _OLY_ENGINE_LOG_WARNING("RENDERING") << "Cannot write texture to using null sprite_ batch" << kLog.nl;
            return;
        }

        dirty_ = false;

        if (debug_group_.shapes.empty())
        {
            sprite_.set_texture(kRefNull);
            return;
        }

        math::Rect2D bounds;
        float rotation = 0.0f;

        if (paint_options.bounds_use_rotation)
        {
            math::RotatedRect2D obb;
            if (debug_group_.shapes.size() == 1)
                obb = debug_group_.shapes[0].rotated_bounds();
            else
            {
                std::vector<glm::vec2> points;
                for (size_t i = 0; i < debug_group_.shapes.size(); ++i)
                {
                    auto pts = debug_group_.shapes[i].rotated_bounds().points();
                    points.insert(points.end(), pts.begin(), pts.end());
                }
                obb = col2d::OBB::fast_wrap(points.data(), points.size()).rect();
            }

            bounds = {
                .x1 = obb.center.x - 0.5f * obb.size.x,
                .x2 = obb.center.x + 0.5f * obb.size.x,
                .y1 = obb.center.y - 0.5f * obb.size.y,
                .y2 = obb.center.y + 0.5f * obb.size.y
            };
            rotation = obb.rotation;
        }
        else
        {
            bounds = debug_group_.shapes[0].bounds();
            for (size_t i = 1; i < debug_group_.shapes.size(); ++i)
                bounds.include_rect(debug_group_.shapes[i].bounds());
        }

        if (bounds.width() <= 0 || bounds.height() <= 0)
        {
            sprite_.set_texture(kRefNull);
            return;
        }

        glm::vec2 scale = paint_options.compress(bounds);
        auto paint_context = layer_->painter_.paint_context(
            sprite_batch_->camera,
            math::IRect2D::round_out(bounds),
            rotation,
            scale);

        for (const auto& shape : debug_group_.shapes)
            shape.draw();

        paint_context.render();
        paint_context.set_texture(sprite_);
        sprite_.set_transform(
            Transform2D{ .position = bounds.center(), .rotation = rotation, .scale = scale }.matrix());
    }

    void DebugOverlay::invalidate_layer()
    {
        layer_ = nullptr;
        debug_group_.shapes.clear();
    }

    DebugOverlayLayer::DebugOverlayLayer(const DebugOverlayLayer& o)
        : painter_(o.painter_)
    {
    }

    DebugOverlayLayer::DebugOverlayLayer(DebugOverlayLayer&& o) noexcept
        : painter_(std::move(o.painter_))
        , collision_views_(std::move(o.collision_views_))
    {
        for (DebugOverlay* view : collision_views_)
        {
            view->layer_ = this;
            view->sprite_.set_batch(sprite_batch_);
        }
    }

    DebugOverlayLayer::~DebugOverlayLayer()
    {
        for (DebugOverlay* view : collision_views_)
            view->invalidate_layer();
    }

    DebugOverlayLayer& DebugOverlayLayer::operator=(const DebugOverlayLayer& o)
    {
        if (this != &o)
        {
            for (DebugOverlay* view : collision_views_)
                view->invalidate_layer();
            collision_views_.clear();

            painter_ = o.painter_;
        }
        return *this;
    }

    DebugOverlayLayer& DebugOverlayLayer::operator=(DebugOverlayLayer&& o) noexcept
    {
        if (this != &o)
        {
            for (DebugOverlay* view : collision_views_)
                view->invalidate_layer();
            collision_views_.clear();

            painter_ = std::move(o.painter_);
            collision_views_ = std::move(o.collision_views_);

            for (DebugOverlay* view : collision_views_)
            {
                view->layer_ = this;
                view->sprite_.set_batch(sprite_batch_);
            }
        }
        return *this;
    }

    const rendering::SpriteBatch& DebugOverlayLayer::get_sprite_batch() const
    {
        return sprite_batch_;
    }

    void DebugOverlayLayer::draw() const
    {
        for (DebugOverlay* view : collision_views_)
            view->draw_sprite();
        sprite_batch_->render();
    }

    DebugShape DebugOverlayLayer::default_debug_shape()
    {
        return DebugShape(rendering::StaticEllipse(painter_.get_ellipse_batch()), *this);
    }

    rendering::StaticEllipse DebugOverlayLayer::create_ellipse()
    {
        return rendering::StaticEllipse(painter_.get_ellipse_batch());
    }

    rendering::StaticPolygon DebugOverlayLayer::create_polygon()
    {
        return rendering::StaticPolygon(painter_.get_polygon_batch());
    }

    rendering::StaticArrowExtension DebugOverlayLayer::create_arrow()
    {
        return rendering::StaticArrowExtension(painter_.get_polygon_batch());
    }

    void DebugOverlayLayer::assign(DebugOverlay* view)
    {
        if (collision_views_.insert(view).second)
            view->sprite_.set_batch(sprite_batch_);
    }

    void DebugOverlayLayer::unassign(DebugOverlay* view)
    {
        if (collision_views_.erase(view))
            view->sprite_.set_batch(rendering::kUnbatched);
    }
}
