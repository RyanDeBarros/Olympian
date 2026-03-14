#include "Collision.h"

#include "physics/collision/scene/dispatch/CollisionDispatcher.h"

namespace oly::context
{
	namespace internal
	{
		std::array<std::string, 32> collision_mask_names, collision_layer_names;
	}
	
	void internal::init_collision(TOMLNode node)
	{
		if (auto collision_node = node["collision"])
		{
			if (auto masks = collision_node["masks"].as_array())
			{
				for (int i = 0; i < std::min((int)masks->size(), 32); ++i)
				{
					if (auto name = masks->get_as<std::string>(i))
						set_collision_mask_index(i, **name);
					else
						_OLY_ENGINE_LOG_WARNING("CONTEXT") << "Collision mask name is not a string for index (" << i << ")." << LOG.nl;
				}
			}

			if (auto layers = collision_node["layers"].as_array())
			{
				for (int i = 0; i < std::min((int)layers->size(), 32); ++i)
				{
					if (auto name = layers->get_as<std::string>(i))
						set_collision_layer_index(i, **name);
					else
						_OLY_ENGINE_LOG_WARNING("CONTEXT") << "Collision layer name is not a string for index (" << i << ")." << LOG.nl;
				}
			}
		}

		col2d::internal::load_luts();
		col2d::CollisionDispatcher::instance();
	}

	col2d::Mask get_collision_mask(const StringParam& name)
	{
		if (name.empty())
			return 0;
		auto it = std::find(internal::collision_mask_names.begin(), internal::collision_mask_names.end(), name);
		if (it != internal::collision_mask_names.end())
			return col2d::Mask(1 << (it - internal::collision_mask_names.begin()));
		else
			return 0;
	}

	void set_collision_mask_index(int index, const StringParam& name)
	{
		if (index >= 0 && index < 32)
		{
			if (!name.empty())
				for (int i = 0; i < internal::collision_mask_names.size(); ++i)
					if (i != index && internal::collision_mask_names[i] == name)
						throw Error(ErrorCode::DuplicateKey);

			internal::collision_mask_names[index] = name.transfer();
		}
		else
			throw Error(ErrorCode::IndexOutOfRange);
	}

	col2d::Layer get_collision_layer(const StringParam& name)
	{
		if (name.empty())
			return 0;
		auto it = std::find(internal::collision_layer_names.begin(), internal::collision_layer_names.end(), name);
		if (it != internal::collision_layer_names.end())
			return col2d::Layer(1 << (it - internal::collision_layer_names.begin()));
		else
			return 0;
	}

	void set_collision_layer_index(int index, const StringParam& name)
	{
		if (index >= 0 && index < 32)
		{
			if (!name.empty())
				for (int i = 0; i < internal::collision_layer_names.size(); ++i)
					if (i != index && internal::collision_layer_names[i] == name)
						throw Error(ErrorCode::DuplicateKey);

			internal::collision_layer_names[index] = name.transfer();
		}
		else
			throw Error(ErrorCode::IndexOutOfRange);
	}
}
