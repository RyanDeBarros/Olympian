#pragma once

#include "physics/collision/debugging/CoreViews.h"

namespace oly::debug
{
	inline CollisionView collision_view(const col2d::ContactResult::Feature& feature, glm::vec4 color, float arrow_width = 6.0f)
	{
		rendering::StaticArrowExtension impulse;
		impulse.set_color(color);
		impulse.adjust_standard_head_for_width(arrow_width);
		impulse.set_start() = feature.position;
		impulse.set_end() = feature.position + feature.impulse;
		return CollisionView(std::move(impulse));
	}

	inline void update_view(CollisionView& view, const col2d::ContactResult::Feature& feature, glm::vec4 color, float arrow_width = 6.0f, size_t view_index = 0)
	{
		CollisionObject& obj = view.get_view(view_index);
		bool modify = std::visit([](const auto& obj) -> bool {
			if constexpr (visiting_class_is<decltype(obj), rendering::StaticArrowExtension>)
				return true;
			else
				return false;
			}, obj);

		if (modify)
		{
			rendering::StaticArrowExtension& impulse = std::get<CollisionObjectType::ARROW>(obj);
			impulse.set_color(color);
			impulse.adjust_standard_head_for_width(arrow_width);
			impulse.set_start() = feature.position;
			impulse.set_end() = feature.position + feature.impulse;
			view.view_changed();
		}
		else
		{
			rendering::StaticArrowExtension impulse;
			impulse.set_color(color);
			impulse.adjust_standard_head_for_width(arrow_width);
			impulse.set_start() = feature.position;
			impulse.set_end() = feature.position + feature.impulse;
			view.set_view(std::move(impulse));
		}
	}

	inline void update_view_no_color(CollisionView& view, const col2d::ContactResult::Feature& feature, float arrow_width = 6.0f, size_t view_index = 0)
	{
		CollisionObject& obj = view.get_view(view_index);
		bool modify = std::visit([](const auto& obj) -> bool {
			if constexpr (visiting_class_is<decltype(obj), rendering::StaticArrowExtension>)
				return true;
			else
				return false;
			}, obj);

		if (modify)
		{
			rendering::StaticArrowExtension& impulse = std::get<CollisionObjectType::ARROW>(obj);
			impulse.adjust_standard_head_for_width(arrow_width);
			impulse.set_start() = feature.position;
			impulse.set_end() = feature.position + feature.impulse;
			view.view_changed();
		}
		else
		{
			rendering::StaticArrowExtension impulse;
			impulse.adjust_standard_head_for_width(arrow_width);
			impulse.set_start() = feature.position;
			impulse.set_end() = feature.position + feature.impulse;
			view.set_view(std::move(impulse));
		}
	}

	constexpr float INFINITE_RAY_LENGTH = 1'000'000.0f;

	inline CollisionView collision_view(const col2d::Ray& ray, glm::vec4 color, float arrow_width = 6.0f)
	{
		rendering::StaticArrowExtension arrow;
		arrow.set_color(color);
		arrow.adjust_standard_head_for_width(arrow_width);
		arrow.set_start() = ray.origin;
		float clip = ray.clip == 0.0f ? INFINITE_RAY_LENGTH : ray.clip;
		arrow.set_end() = ray.origin + clip * (glm::vec2)ray.direction;
		return CollisionView(std::move(arrow));
	}

	inline void update_view(CollisionView& view, const col2d::Ray& ray, glm::vec4 color, float arrow_width = 6.0f, size_t view_index = 0)
	{
		CollisionObject& obj = view.get_view(view_index);
		bool modify = std::visit([](const auto& obj) -> bool {
			if constexpr (visiting_class_is<decltype(obj), rendering::StaticArrowExtension>)
				return true;
			else
				return false;
			}, obj);

		if (modify)
		{
			rendering::StaticArrowExtension& arrow = std::get<CollisionObjectType::ARROW>(obj);
			arrow.set_color(color);
			arrow.adjust_standard_head_for_width(arrow_width);
			arrow.set_start() = ray.origin;
			arrow.set_end() = ray.clip == 0.0f ? (ray.origin + INFINITE_RAY_LENGTH * (glm::vec2)ray.direction) : ray.origin + ray.clip * (glm::vec2)ray.direction;
			view.view_changed();
		}
		else
		{
			rendering::StaticArrowExtension arrow;
			arrow.set_color(color);
			arrow.adjust_standard_head_for_width(arrow_width);
			arrow.set_start() = ray.origin;
			arrow.set_end() = ray.clip == 0.0f ? (ray.origin + INFINITE_RAY_LENGTH * (glm::vec2)ray.direction) : ray.origin + ray.clip * (glm::vec2)ray.direction;
			view.set_view(std::move(arrow));
		}
	}

	inline void update_view_no_color(CollisionView& view, const col2d::Ray& ray, float arrow_width = 6.0f, size_t view_index = 0)
	{
		CollisionObject& obj = view.get_view(view_index);
		bool modify = std::visit([](const auto& obj) -> bool {
			if constexpr (visiting_class_is<decltype(obj), rendering::StaticArrowExtension>)
				return true;
			else
				return false;
			}, obj);

		if (modify)
		{
			rendering::StaticArrowExtension& arrow = std::get<CollisionObjectType::ARROW>(obj);
			arrow.adjust_standard_head_for_width(arrow_width);
			arrow.set_start() = ray.origin;
			arrow.set_end() = ray.clip == 0.0f ? (ray.origin + INFINITE_RAY_LENGTH * (glm::vec2)ray.direction) : ray.origin + ray.clip * (glm::vec2)ray.direction;
			view.view_changed();
		}
		else
		{
			rendering::StaticArrowExtension arrow;
			arrow.adjust_standard_head_for_width(arrow_width);
			arrow.set_start() = ray.origin;
			arrow.set_end() = ray.clip == 0.0f ? (ray.origin + INFINITE_RAY_LENGTH * (glm::vec2)ray.direction) : ray.origin + ray.clip * (glm::vec2)ray.direction;
			view.set_view(std::move(arrow));
		}
	}

	inline CollisionView collision_view(const col2d::RaycastResult& result, glm::vec4 color, float impulse_length = 50.0f, float arrow_width = 6.0f)
	{
		if (result.hit == col2d::RaycastResult::Hit::TRUE_HIT)
		{
			col2d::Ray ray{ .origin = result.contact, .direction = result.normal, .clip = impulse_length };
			return collision_view(ray, color, arrow_width);
		}
		else
			return CollisionView();
	}

	inline void update_view(CollisionView& view, const col2d::RaycastResult& result, glm::vec4 color, float impulse_length = 50.0f, float arrow_width = 6.0f, size_t view_index = 0)
	{
		if (result.hit == col2d::RaycastResult::Hit::TRUE_HIT)
		{
			col2d::Ray ray{ .origin = result.contact, .direction = result.normal, .clip = impulse_length };
			update_view(view, ray, color, arrow_width, view_index);
		}
		else
			view.clear_view();
	}

	inline void update_view_no_color(CollisionView& view, const col2d::RaycastResult& result, float impulse_length = 50.0f, float arrow_width = 6.0f, size_t view_index = 0)
	{
		if (result.hit == col2d::RaycastResult::Hit::TRUE_HIT)
		{
			col2d::Ray ray{ .origin = result.contact, .direction = result.normal, .clip = impulse_length };
			update_view_no_color(view, ray, arrow_width, view_index);
		}
		else
			view.clear_view();
	}

	inline CollisionView collision_view(const col2d::RectCast& cast, glm::vec4 color, glm::vec4 arrow_color)
	{
		CollisionView view = collision_view(cast.finite_obb(INFINITE_RAY_LENGTH), color);
		view.merge(collision_view(cast.ray, arrow_color));
		return view;
	}

	inline void update_view(CollisionView& view, const col2d::RectCast& cast, glm::vec4 color, glm::vec4 arrow_color, float arrow_width = 6.0f, size_t view_index = 0)
	{
		view.resize_view(std::max(view_index + 2, view.view_size()));
		update_view(view, cast.finite_obb(INFINITE_RAY_LENGTH), color, view_index);
		update_view(view, cast.ray, arrow_color, arrow_width, view_index + 1);
	}

	inline void update_view_no_color(CollisionView& view, const col2d::RectCast& cast, float arrow_width = 6.0f, size_t view_index = 0)
	{
		view.resize_view(std::max(view_index + 2, view.view_size()));
		update_view_no_color(view, cast.finite_obb(INFINITE_RAY_LENGTH), view_index);
		update_view_no_color(view, cast.ray, arrow_width, view_index + 1);
	}

	inline CollisionView collision_view(const col2d::CircleCast& cast, glm::vec4 color, glm::vec4 arrow_color)
	{
		CollisionView view = collision_view(cast.finite_capsule(INFINITE_RAY_LENGTH).compound(), color);
		view.merge(collision_view(cast.ray, arrow_color));
		return view;
	}

	inline void update_view(CollisionView& view, const col2d::CircleCast& cast, glm::vec4 color, glm::vec4 arrow_color, float arrow_width = 6.0f, size_t view_index = 0)
	{
		view.resize_view(std::max(view_index + 4, view.view_size()));
		update_view(view, cast.finite_capsule(INFINITE_RAY_LENGTH).mid_obb(), color, view_index);
		update_view(view, cast.finite_capsule(INFINITE_RAY_LENGTH).lower_circle(), color, view_index + 1);
		update_view(view, cast.finite_capsule(INFINITE_RAY_LENGTH).upper_circle(), color, view_index + 2);
		update_view(view, cast.ray, arrow_color, arrow_width, view_index + 3);
	}

	inline void update_view_no_color(CollisionView& view, const col2d::CircleCast& cast, float arrow_width = 6.0f, size_t view_index = 0)
	{
		view.resize_view(std::max(view_index + 4, view.view_size()));
		update_view_no_color(view, cast.finite_capsule(INFINITE_RAY_LENGTH).mid_obb(), view_index);
		update_view_no_color(view, cast.finite_capsule(INFINITE_RAY_LENGTH).lower_circle(), view_index + 1);
		update_view_no_color(view, cast.finite_capsule(INFINITE_RAY_LENGTH).upper_circle(), view_index + 2);
		update_view_no_color(view, cast.ray, arrow_width, view_index + 3);
	}
}
