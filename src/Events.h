#pragma once

#include "TreeNode.h"

namespace oly
{
	template<typename EventData>
	struct EventHandler : public TreeNode<EventHandler<EventData>>
	{
		using TreeNode<EventHandler<EventData>>::TreeNode;

		virtual bool block(const EventData& data) { return false; }
		virtual bool consume(const EventData& data) { return false; }

		bool handle(const EventData& data)
		{
			if (block(data))
				return false;
			for (auto child : *this)
			{
				if (child->handle(data))
					return true;
			}
			return consume(data);
		}
	};
}
