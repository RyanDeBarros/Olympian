#pragma once

#include <memory>
#include <unordered_map>

#include <imp/type_erasure.hpp>

namespace oly::editor
{
	class IPanel;

	class PanelManager
	{
		std::unordered_map<imp::type_erasure, std::unique_ptr<IPanel>> _panels;

	public:
		void Init();
		void Draw();

		template<typename T>
		T& Add()
		{
			IPanel& panel = Add(imp::erase_type<T>(), std::make_unique<T>());
			return static_cast<T&>(panel);
		}

		IPanel& Add(imp::type_erasure index, std::unique_ptr<IPanel>&& panel);

		template<typename T>
		T* Get()
		{
			return static_cast<T*>(Get(imp::erase_type<T>()));
		}

		IPanel* Get(imp::type_erasure index);
	};
}
