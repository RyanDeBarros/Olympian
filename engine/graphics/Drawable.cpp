#include "Drawable.h"

namespace oly::rendering
{
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
		void CanvasLayerStackBase::draw_tree(const IDrawable& stack) const
		{
			if (dirty)
			{
				dirty = false;
				layers.clear();
				append_layers(stack);
			}

			stack.on_draw();

			auto layer = layers.begin();
			while (layer != layers.end() && layer->first < 0.0f)
			{
				for (const IDrawable* d : layer->second)
					d->on_draw();
				++layer;
			}

			for (const IDrawable& d : stack)
				draw_subtree(d);

			while (layer != layers.end())
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
				layers[zd->get_z_layer()].push_back(zd);

			for (const IDrawable& d : drawable)
				append_layers(d);
		}
	}
}
