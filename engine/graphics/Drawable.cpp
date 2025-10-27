#include "Drawable.h"

namespace oly::rendering
{
	void CanvasLayerStack::draw_tree() const
	{
		if (dirty)
		{
			dirty = false; // TODO v5 set dirty when attaching children - check in IDrawable? Or create a new class that implements z-layer - not all IDrawables. This new class would hold a reference to the canvases it's a part of, and will update their map when layer is changed.
			gen_layer_map();
		}

		on_draw();

		auto layer = layers.begin();
		while (layer != layers.end() && layer->first < 0.0f)
		{
			for (const IDrawable* d : layer->second)
				d->on_draw();
			++layer;
		}

		for (const IDrawable& d : *this)
			draw_subtree(d);

		while (layer != layers.end())
		{
			for (const IDrawable* d : layer->second)
				d->on_draw();
			++layer;
		}
	}

	void CanvasLayerStack::draw_subtree(const IDrawable& drawable) const
	{
		auto zd = dynamic_cast<const ZLayeredDrawable*>(&drawable);
		if (!zd || zd->get_z_layer() == 0.0f)
			drawable.on_draw();

		for (const IDrawable& d : drawable)
			draw_subtree(d);
	}

	void CanvasLayerStack::gen_layer_map() const
	{
		layers.clear();
		append_layers(*this);
	}

	void CanvasLayerStack::append_layers(const IDrawable& drawable) const
	{
		auto zd = dynamic_cast<const ZLayeredDrawable*>(&drawable);
		if (zd && zd->get_z_layer() != 0.0f)
			layers[zd->get_z_layer()].push_back(zd);

		for (const IDrawable& d : drawable)
			append_layers(d);
	}
}
