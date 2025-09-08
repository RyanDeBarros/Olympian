#include "Collision.h"

#include "physics/collision/scene/dispatch/CollisionDispatcher.h"

namespace oly::context
{
	namespace internal
	{
		col2d::internal::CollisionDispatcher collision_dispatcher;
		std::array<std::string, 32> collision_mask_names, collision_layer_names;
	}

	void internal::init_collision(const TOMLNode& node)
	{
		// TODO v4 add to editor
		if (auto collision_node = node["collision"])
		{
			if (auto masks = collision_node["masks"].as_array())
			{
				for (int i = 0; i < std::min((int)masks->size(), 32); ++i)
				{
					if (auto name = masks->get_as<std::string>(i))
						set_collision_mask_index(i, **name);
					else
						OLY_LOG_WARNING(true, "CONTEXT") << LOG.source_info.full_source() << "Collision mask name is not a string for index (" << i << ")." << LOG.nl;
				}
			}

			if (auto layers = collision_node["layers"].as_array())
			{
				for (int i = 0; i < std::min((int)layers->size(), 32); ++i)
				{
					if (auto name = layers->get_as<std::string>(i))
						set_collision_layer_index(i, **name);
					else
						OLY_LOG_WARNING(true, "CONTEXT") << LOG.source_info.full_source() << "Collision layer name is not a string for index (" << i << ")." << LOG.nl;
				}
			}
		}
	}

	void internal::terminate_collision()
	{
		internal::collision_dispatcher.clear();
	}

	void internal::frame_collision()
	{
		internal::collision_dispatcher.poll();
	}

	col2d::internal::CollisionDispatcher& collision_dispatcher()
	{
		return internal::collision_dispatcher;
	}

	col2d::Mask get_collision_mask(const std::string& name)
	{
		auto it = std::find(internal::collision_mask_names.begin(), internal::collision_mask_names.end(), name);
		if (it != internal::collision_mask_names.end())
			return col2d::Mask(1 << (it - internal::collision_mask_names.begin()));
		else
			return 0;
	}

	void set_collision_mask(col2d::Mask mask, const std::string& name)
	{
		for (int i = 0; i < internal::collision_mask_names.size(); ++i)
			if ((1 << i) != mask && internal::collision_mask_names[i] == name)
				throw Error(ErrorCode::DUPLICATE_KEY);

		internal::collision_mask_names[mask] = name;
	}

	void set_collision_mask_index(int index, const std::string& name)
	{
		if (index >= 0 && index < 32)
			set_collision_mask(col2d::Mask(1 << index), name);
		else
			throw Error(ErrorCode::INDEX_OUT_OF_RANGE);
	}

	col2d::Layer get_collision_layer(const std::string& name)
	{
		auto it = std::find(internal::collision_layer_names.begin(), internal::collision_layer_names.end(), name);
		if (it != internal::collision_layer_names.end())
			return col2d::Layer(1 << (it - internal::collision_layer_names.begin()));
		else
			return 0;
	}

	void set_collision_layer(col2d::Layer layer, const std::string& name)
	{
		for (int i = 0; i < internal::collision_layer_names.size(); ++i)
			if ((1 << i) != layer && internal::collision_layer_names[i] == name)
				throw Error(ErrorCode::DUPLICATE_KEY);

		internal::collision_layer_names[layer] = name;
	}

	void set_collision_layer_index(int index, const std::string& name)
	{
		if (index >= 0 && index < 32)
			set_collision_layer(col2d::Layer(1 << index), name);
		else
			throw Error(ErrorCode::INDEX_OUT_OF_RANGE);
	}
}
