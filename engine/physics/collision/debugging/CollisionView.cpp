#include "CollisionView.h"

#include "core/base/Context.h"

namespace oly::debug
{
	void internal::CollisionViewImpl::render() const
	{
		for (const auto& obj : objects)
			std::visit([](auto&& obj) { obj.draw(); }, obj);
		objects.clear();
		// TODO should ellipses/polygons be rendered in bulk at end like here, or should they be rendered in order of objects vector, meaning that the current batch type must be tracked when iterating over objects ?
		context::render_ellipses();
		context::render_polygons();
	}
}
