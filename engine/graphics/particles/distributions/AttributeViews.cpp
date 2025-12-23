#include "AttributeViews.h"

namespace oly::particles
{
	void SineAttributeView::on_tick()
	{
		attribute = a * glm::sin(b * emitter.time_elapsed() - k) + c;
	}
}
