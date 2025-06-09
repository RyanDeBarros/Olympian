#include "CollisionView.h"

#include "core/base/Context.h"

namespace oly::debug
{
	void CollisionView::draw()
	{
		static const auto draw_object = [](auto&& obj) { std::visit([](auto&& obj) { obj.draw(); }, obj); };
		std::visit([](auto&& view) {
			if constexpr (std::is_same_v<std::decay_t<decltype(view)>, Object>)
				draw_object(view);
			else if constexpr (std::is_same_v<std::decay_t<decltype(view)>, std::vector<Object>>)
				for (const auto& obj : view)
					draw_object(obj);
			}, obj);
	}

	void CollisionView::merge(CollisionView&& other)
	{
		if (other.obj.index() == EMPTY)
			return;

		if (obj.index() == SINGLE)
		{
			Object old_view = std::move(std::get<SINGLE>(obj));
			obj = std::vector<Object>();
			std::get<VECTOR>(obj).push_back(std::move(old_view));
		}

		std::visit([&view = std::get<VECTOR>(obj)](auto&& other_view) {
			if constexpr (std::is_same_v<std::decay_t<decltype(other_view)>, Object>)
				view.push_back(std::move(other_view));
			else if constexpr (std::is_same_v<std::decay_t<decltype(other_view)>, std::vector<Object>>)
				view.insert(view.end(), std::make_move_iterator(other_view.begin()), std::make_move_iterator(other_view.end()));
			}, std::move(other.obj));
	}

	void render_collision()
	{
		context::render_ellipses();
		context::render_polygons();
	}
}
