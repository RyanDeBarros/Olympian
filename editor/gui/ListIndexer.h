#pragma once

#include <imtk.hpp>

#include <imp/counter.hpp>

namespace oly::editor
{
	struct ListIndexer : public imtk::w::owned_list_indexer
	{
		struct Ctor
		{
			std::string prompt;
			std::string create_tooltip;
			std::string delete_tooltip;
			std::string clear_tooltip;
		};

		ListIndexer(Ctor config, std::function<std::string(size_t)> combo_name);
		ListIndexer(Ctor config, std::string combo_slot_prefix);
	};
}
