#include "InputController.h"

#include "core/context/Platform.h"

namespace oly
{
	InputController::InputController()
	{
		context::input_binding_context().controller_signals[this];
	}

	InputController::InputController(const InputController& other)
	{
		context::input_binding_context().controller_signals[this];
		// don't copy signals
	}

	InputController::InputController(InputController&& other) noexcept
	{
		move_controller(std::move(other));
	}

	InputController::~InputController()
	{
		erase_controller();
	}

	InputController& InputController::operator=(const InputController& other)
	{
		if (this != &other)
		{
			erase_controller();
			// don't copy signals
			context::input_binding_context().controller_signals.find(this)->second.clear();
		}
		return *this;
	}

	InputController& InputController::operator=(InputController&& other) noexcept
	{
		if (this != &other)
		{
			erase_controller();
			move_controller(std::move(other));
		}
		return *this;
	}

	void InputController::move_controller(InputController&& other) const
	{
		auto& binding_context = context::input_binding_context();
		auto& signals = (binding_context.controller_signals[this] = std::move(binding_context.controller_signals.find(&other)->second));
		auto& handler_map = binding_context.handler_map;
		for (input::SignalID signal : signals)
			handler_map.find(signal)->second->controller = this;
	}

	void InputController::erase_controller() const
	{
		auto& binding_context = context::input_binding_context();
		auto& signals = binding_context.controller_signals.find(this)->second;
		auto& handler_map = binding_context.handler_map;
		for (input::SignalID signal : signals)
			handler_map.erase(signal);
	}

	void InputController::insert_signal(input::SignalID signal) const
	{
		context::input_binding_context().controller_signals.find(this)->second.insert(signal);
	}

	void InputController::erase_signal(input::SignalID signal) const
	{
		context::input_binding_context().controller_signals.find(this)->second.erase(signal);
	}

	void InputController::bind(input::SignalID signal, Handler handler)
	{
		auto& binding_context = context::input_binding_context();
		auto& handler_map = binding_context.handler_map;
		auto it = handler_map.find(signal);
		if (it != handler_map.end())
		{
			const InputController* existing_controller = it->second->controller;
			if (existing_controller != this)
			{
				existing_controller->erase_signal(signal);
				insert_signal(signal);
			}
			it->second = std::make_unique<input::internal::InputBindingContext::HandlerRef>(*this, handler);
		}
		else
		{
			insert_signal(signal);
			handler_map[signal] = std::make_unique<input::internal::InputBindingContext::HandlerRef>(*this, handler);
		}
	}

	void InputController::bind(input::SignalID signal, ConstHandler handler) const
	{
		auto& binding_context = context::input_binding_context();
		auto& handler_map = binding_context.handler_map;
		auto it = handler_map.find(signal);
		if (it != handler_map.end())
		{
			const InputController* existing_controller = it->second->controller;
			if (existing_controller != this)
			{
				existing_controller->erase_signal(signal);
				insert_signal(signal);
			}
			it->second = std::make_unique<input::internal::InputBindingContext::ConstHandlerRef>(*this, handler);
		}
		else
		{
			insert_signal(signal);
			handler_map[signal] = std::make_unique<input::internal::InputBindingContext::ConstHandlerRef>(*this, handler);
		}
	}

	void InputController::unbind(input::SignalID signal) const
	{
		auto& binding_context = context::input_binding_context();
		auto& handler_map = binding_context.handler_map;
		auto it = handler_map.find(signal);
		if (it != handler_map.end() && it->second->controller == this)
		{
			binding_context.controller_signals.find(this)->second.erase(signal);
			handler_map.erase(it);
		}
	}

	void InputController::bind(const std::string& signal, Handler handler)
	{
		bind(context::signal_table().get(signal), handler);
	}

	void InputController::bind(const std::string& signal, ConstHandler handler) const
	{
		bind(context::signal_table().get(signal), handler);
	}

	void InputController::unbind(const std::string& signal) const
	{
		unbind(context::signal_table().get(signal));
	}

	void InputController::bind_mapping(const std::string& mapping, Handler handler)
	{
		const std::vector<std::string>& signals = context::signal_mapping_table()[mapping];
		for (const std::string& signal : signals)
			bind(signal, handler);
	}

	void InputController::bind_mapping(const std::string& mapping, ConstHandler handler) const
	{
		const std::vector<std::string>& signals = context::signal_mapping_table()[mapping];
		for (const std::string& signal : signals)
			bind(signal, handler);
	}

	void InputController::unbind_mapping(const std::string& mapping) const
	{
		const std::vector<std::string>& signals = context::signal_mapping_table()[mapping];
		for (const std::string& signal : signals)
			unbind(signal);
	}
}
