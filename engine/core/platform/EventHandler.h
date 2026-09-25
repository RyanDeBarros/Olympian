#pragma once

#include <imp/tree_node.hpp>

namespace oly
{
	template<typename EventData>
	struct EventHandler : public imp::tree_node<EventHandler<EventData>>
	{
		using imp::tree_node<EventHandler<EventData>>::tree_node;

		virtual bool block(const EventData& data) { return false; }
		virtual bool consume(const EventData& data) { return false; }

		bool handle(const EventData& data)
		{
			if (block(data))
				return false;

			for (EventHandler<EventData>& child : *this)
				if (child.handle(data))
					return true;

			return consume(data);
		}
	};
}
