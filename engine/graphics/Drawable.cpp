#include "Drawable.h"

namespace oly::rendering
{
	void IDrawable::draw() const
	{
		LayerMap layers = gen_layer_map(); // TODO v5 cache layers
		for (const auto& [z, v] : layers)
			for (const IDrawable* d : v)
				d->on_draw();
	}

	IDrawable::LayerMap IDrawable::gen_layer_map() const
	{
		LayerMap layers;
		append_layers(layers);
		return layers;
	}

	void IDrawable::append_layers(LayerMap& layers) const
	{
		layers[z_order].push_back(this);
		for (const IDrawable* d : *this)
			d->append_layers(layers);
	}
}
