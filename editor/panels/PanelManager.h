#pragma once

#include <memory>
#include <unordered_map>
#include <typeindex>

class IPanel;

class PanelManager
{
	std::unordered_map<std::type_index, std::unique_ptr<IPanel>> _panels;

public:
	void Draw();

	template<typename T>
	T& Add()
	{
		IPanel& panel = Add(typeid(T), std::make_unique<T>());
		return static_cast<T&>(panel);
	}

	IPanel& Add(std::type_index index, std::unique_ptr<IPanel>&& panel);

    template<typename T>
    T* Get()
    {
        return static_cast<T*>(Get(typeid(T)));
    }

    IPanel* Get(std::type_index index);
};
