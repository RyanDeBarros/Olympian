#pragma once

#include "core/platform/Signal.h"

#include "assets/ResourcePath.h"

namespace oly
{
    namespace input::internal
    {
        class InputBindingContext;
    }

    class InputController
    {
        input::SignalTable signal_table_;
        input::SignalRoutingTable signal_routing_table_;

    public:
        InputController();
        InputController(const InputController&) = delete;
        InputController(InputController&&) noexcept = delete;
        virtual ~InputController();

    public:
        using Handler = bool(InputController::*)(input::Signal);
        using ConstHandler = bool(InputController::*)(input::Signal) const;

        void bind(input::SignalID signal, Handler handler);
        void bind(input::SignalID signal, ConstHandler handler) const;
        void unbind(input::SignalID signal) const;

        void bind(const StringParam& signal, Handler handler);
        void bind(const StringParam& signal, ConstHandler handler) const;
        void unbind(const StringParam& signal) const;

        template<std::derived_from<InputController> Controller>
        void bind(const StringParam& signal, bool(Controller::* handler)(input::Signal))
        {
            bind(signal, static_cast<Handler>(handler));
        }

        template<std::derived_from<InputController> Controller>
        void bind(const StringParam& signal, bool(Controller::* handler)(input::Signal) const) const
        {
            bind(signal, static_cast<ConstHandler>(handler));
        }

        void load_signals(const detail::ResourcePath& file);

    private:
        void insert_signal(input::SignalID) const;
        void erase_signal(input::SignalID) const;
    };
}
