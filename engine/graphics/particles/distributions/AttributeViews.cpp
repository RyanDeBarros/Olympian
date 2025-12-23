#include "AttributeViews.h"

namespace oly::particles
{
	void GenericAttributeView1D::on_tick()
	{
		fn(emitter, attribute);
	}

	void SineAttributeView1D::on_tick()
	{
		attribute = a * glm::sin(b * emitter.time_elapsed() - k) + c;
	}

	void GenericAttributeView2D::on_tick()
	{
		fn(emitter, attribute);
	}

	void GenericAttributeView3D::on_tick()
	{
		fn(emitter, attribute);
	}

	void GenericAttributeView4D::on_tick()
	{
		fn(emitter, attribute);
	}
}
