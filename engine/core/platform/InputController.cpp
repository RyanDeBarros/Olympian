#include "InputController.h"

#include "core/context/Platform.h"

namespace oly
{
	InputController::InputController()
	{
		context::input_binding_context().controller_lut[this];
	}

	InputController::~InputController()
	{
		auto& binding_context = context::input_binding_context();
		auto& signals = binding_context.controller_lut.find(this)->second;
		for (input::SignalID signal : signals)
			binding_context.handler_map.erase(signal);
	}

	void InputController::insert_signal(input::SignalID signal) const
	{
		context::input_binding_context().controller_lut.find(this)->second.insert(signal);
	}

	void InputController::erase_signal(input::SignalID signal) const
	{
		context::input_binding_context().controller_lut.find(this)->second.erase(signal);
	}

	void InputController::bind(input::SignalID signal, Handler handler)
	{
		auto& binding_context = context::input_binding_context();
		auto it = binding_context.handler_map.find(signal);
		if (it != binding_context.handler_map.end())
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
			binding_context.handler_map[signal] = std::make_unique<input::internal::InputBindingContext::HandlerRef>(*this, handler);
		}
	}

	void InputController::bind(input::SignalID signal, ConstHandler handler) const
	{
		auto& binding_context = context::input_binding_context();
		auto it = binding_context.handler_map.find(signal);
		if (it != binding_context.handler_map.end())
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
			binding_context.handler_map[signal] = std::make_unique<input::internal::InputBindingContext::ConstHandlerRef>(*this, handler);
		}
	}

	void InputController::unbind(input::SignalID signal) const
	{
		auto& binding_context = context::input_binding_context();
		auto it = binding_context.handler_map.find(signal);
		if (it != binding_context.handler_map.end() && it->second->controller == this)
		{
			binding_context.controller_lut.find(this)->second.erase(signal);
			binding_context.handler_map.erase(it);
		}
	}

	void InputController::bind(const StringParam& signal, Handler handler)
	{
		bind(context::signal_table().get(signal), handler);
	}

	void InputController::bind(const StringParam& signal, ConstHandler handler) const
	{
		bind(context::signal_table().get(signal), handler);
	}

	void InputController::unbind(const StringParam& signal) const
	{
		unbind(context::signal_table().get(signal));
	}

	void InputController::bind_mapping(const StringParam& mapping, Handler handler)
	{
		auto it = context::signal_mapping_table().find(mapping);
		if (it != context::signal_mapping_table().end())
		{
			const std::vector<std::string>& signals = it->second;
			for (const std::string& signal : signals)
				bind(signal, handler);
		}
	}

	void InputController::bind_mapping(const StringParam& mapping, ConstHandler handler) const
	{
		auto it = context::signal_mapping_table().find(mapping);
		if (it != context::signal_mapping_table().end())
		{
			const std::vector<std::string>& signals = it->second;
			for (const std::string& signal : signals)
				bind(signal, handler);
		}
	}

	void InputController::unbind_mapping(const StringParam& mapping) const
	{
		auto it = context::signal_mapping_table().find(mapping);
		if (it != context::signal_mapping_table().end())
		{
			const std::vector<std::string>& signals = it->second;
			for (const std::string& signal : signals)
				unbind(signal);
		}
	}
}
