#include "Drawable.h"

namespace oly::rendering
{
    void IDrawable::draw() const
    {
        draw_tree();
    }

    void IDrawable::draw_tree() const
    {
        on_draw();
        for (const IDrawable& d : *this)
            d.draw_tree();
    }

    float ZLayeredDrawable::get_z_layer() const
    {
        return z_layer_;
    }

    void ZLayeredDrawable::set_z_layer(float z)
    {
        z_layer_ = z;
        flag_parent_canvas(get_parent());
    }

    void ZLayeredDrawable::on_attach(IDrawable* old_parent, IDrawable* new_parent)
    {
        flag_parent_canvas(old_parent);
        flag_parent_canvas(new_parent);
    }

    void ZLayeredDrawable::flag_parent_canvas(IDrawable* parent)
    {
        while (parent)
        {
            if (auto stack = dynamic_cast<internal::CanvasLayerStackBase*>(parent))
            {
                stack->mark_dirty();
                break;
            }

            parent = parent->get_parent();
        }
    }

    namespace internal
    {
        void CanvasLayerStackBase::mark_dirty()
        {
            dirty_ = true;
        }

        void CanvasLayerStackBase::draw_tree(const IDrawable& stack) const
        {
            if (dirty_)
            {
                dirty_ = false;
                layers_.clear();
                append_layers(stack);
            }

            stack.on_draw();

            auto layer = layers_.begin();
            while (layer != layers_.end() && layer->first < 0.0f)
            {
                for (const IDrawable* d : layer->second)
                    d->on_draw();

                ++layer;
            }

            for (const IDrawable& d : stack)
                draw_subtree(d);

            while (layer != layers_.end())
            {
                for (const IDrawable* d : layer->second)
                    d->on_draw();

                ++layer;
            }
        }

        void CanvasLayerStackBase::draw_subtree(const IDrawable& drawable) const
        {
            auto zd = dynamic_cast<const ZLayeredDrawable*>(&drawable);
            if (!zd || zd->get_z_layer() == 0.0f)
                drawable.on_draw();

            for (const IDrawable& d : drawable)
                draw_subtree(d);
        }

        void CanvasLayerStackBase::append_layers(const IDrawable& drawable) const
        {
            auto zd = dynamic_cast<const ZLayeredDrawable*>(&drawable);
            if (zd && zd->get_z_layer() != 0.0f)
                layers_[zd->get_z_layer()].push_back(zd);

            for (const IDrawable& d : drawable)
                append_layers(d);
        }
    }
}
