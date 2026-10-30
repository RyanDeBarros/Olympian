#pragma once

#include <imp/type_erasure.hpp>

#include <memory>
#include <unordered_map>

namespace oly::editor
{
    class IPanel;

    class PanelManager
    {
        std::unordered_map<imp::type_erasure, std::unique_ptr<IPanel>> panels_;

    public:
        void Init();
        void Draw();

        IPanel& Add(imp::type_erasure index, std::unique_ptr<IPanel>&& panel);
        IPanel* Get(imp::type_erasure index);

        template<typename T>
        T& Add()
        {
            IPanel& panel = Add(imp::erase_type<T>(), std::make_unique<T>());
            return static_cast<T&>(panel);
        }

        template<typename T>
        T* Get()
        {
            return static_cast<T*>(Get(imp::erase_type<T>()));
        }
    };
}
