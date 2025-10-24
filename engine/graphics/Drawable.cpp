#include "Drawable.h"

namespace oly::rendering
{
	void CanvasLayerStack::draw_tree() const
	{
		gen_layer_map(); // TODO v5 don't regenerate on every draw call
		for (const auto& [z, v] : layers)
			for (const IDrawable* d : v)
				d->on_draw();
	}

	void CanvasLayerStack::gen_layer_map() const
	{
		layers.clear();
		append_layers(this);
	}

	void CanvasLayerStack::append_layers(const IDrawable* drawable) const
	{
		layers[drawable->z_order].push_back(drawable);
		for (const IDrawable* d : *drawable)
			append_layers(d);
	}
}
