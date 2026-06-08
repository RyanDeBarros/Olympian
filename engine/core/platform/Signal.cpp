#include "Signal.h"

namespace oly::input
{
	SignalID SignalTable::gen()
	{
		return _next++;
	}

	void SignalTable::route(std::string name, SignalID id)
	{
		_table[std::move(name)] = id;
	}

	void SignalTable::route(std::string name, std::string base)
	{
		_routes[std::move(name)].push_back(std::move(base));
	}

	SignalID SignalTable::route_new(std::string name)
	{
		SignalID id = gen();
		route(std::move(name), id);
		return id;
	}

	std::vector<SignalID> SignalTable::get(const std::string& name) const
	{
		std::vector<SignalID> ids;

		auto it = _table.find(name);
		if (it != _table.end())
			ids.push_back(it->second);

		auto rit = _routes.find(name);
		if (rit != _routes.end())
		{
			for (auto it = rit->second.begin(); it != rit->second.end(); ++it)
			{
				auto id = _table.find(*it);
				if (id != _table.end())
					ids.push_back(id->second);
			}
		}

		return ids;
	}
}
