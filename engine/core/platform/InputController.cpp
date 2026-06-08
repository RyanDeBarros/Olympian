#include "InputController.h"

#include "core/context/Platform.h"
#include "core/util/Loader.h"
#include "core/util/Logger.h"
#include "core/util/Parser.h"

#include "assets/MetaSplitter.h"
#include "definitions/Keys.h"
#include "definitions/enums/SignalBindingType.h"

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
		if (auto id = _signal_table.get(signal))
			bind(id, handler);
		else
		{
			auto it = _signal_routing_table.find(signal);
			if (it != _signal_routing_table.end())
			{
				// TODO v12 handle infinite recursion
				for (const std::string& signal : it->second)
					bind(signal, handler);
			}
			else
			{
				_OLY_ENGINE_LOG_ERROR("Input") << "Signal name \"" << signal << "\" is not bound to a signal id or routed to any other signal names" << LOG.nl;
			}
		}
	}

	void InputController::bind(const StringParam& signal, ConstHandler handler) const
	{
		if (auto id = _signal_table.get(signal))
			bind(id, handler);
		else
		{
			auto it = _signal_routing_table.find(signal);
			if (it != _signal_routing_table.end())
			{
				// TODO v12 handle infinite recursion
				for (const std::string& signal : it->second)
					bind(signal, handler);
			}
			else
			{
				_OLY_ENGINE_LOG_ERROR("Input") << "Signal name \"" << signal << "\" is not bound to a signal id or routed to any other signal names" << LOG.nl;
			}
		}
	}

	void InputController::unbind(const StringParam& signal) const
	{
		if (auto id = _signal_table.get(signal))
			unbind(id);
		else
		{
			auto it = _signal_routing_table.find(signal);
			if (it != _signal_routing_table.end())
			{
				// TODO v12 handle infinite recursion
				for (const std::string& signal : it->second)
					unbind(signal);
			}
			else
			{
				_OLY_ENGINE_LOG_ERROR("Input") << "Signal name \"" << signal << "\" is not bound to a signal id or routed to any other signal names" << LOG.nl; // TODO v8 don't pass LOG.nl -> log access should do a new line at the beginning, and every frame should flush log -> print any pending newlines.
			}
		}
	}

	static void load_modifier_base(input::ModifierBase& modifier, const assets::Parser& parser)
	{
		parser.optional(detail::Key::Swizzle)(modifier.swizzle);
		parser.optional(detail::Key::Multiplier)(assets::PartialView(modifier.multiplier));
		parser.optional(detail::Key::Invert)(assets::PartialView(modifier.invert));
	}

	static input::Axis0DModifier load_modifier_0d(const assets::Parser& parser)
	{
		input::Axis0DModifier modifier;
		if (auto mod_parser = parser.optional(detail::Key::Modifier).subparser())
		{
			mod_parser->optional(detail::Key::Conversion)(modifier.conversion);
			load_modifier_base(modifier, *mod_parser);
		}
		return modifier;
	}

	static input::Axis1DModifier load_modifier_1d(const assets::Parser& parser)
	{
		input::Axis1DModifier modifier;
		if (auto mod_parser = parser.optional(detail::Key::Modifier).subparser())
		{
			mod_parser->optional(detail::Key::Conversion)(modifier.conversion);
			load_modifier_base(modifier, *mod_parser);
		}
		return modifier;
	}

	static input::Axis2DModifier load_modifier_2d(const assets::Parser& parser)
	{
		input::Axis2DModifier modifier;
		if (auto mod_parser = parser.optional(detail::Key::Modifier).subparser())
		{
			mod_parser->optional(detail::Key::Conversion)(modifier.conversion);
			load_modifier_base(modifier, *mod_parser);
		}
		return modifier;
	}

	static void load_key_binding(input::SignalTable& signal_table, const assets::Parser& parser, const std::string& id)
	{
		input::KeyBinding b;
		if (!parser.optional(detail::Key::Key)(b.key))
			return;
		parser.optional(detail::Key::RequiredMods)(b.required_key_mods);
		parser.optional(detail::Key::ForbiddenMods)(b.forbidden_key_mods);
		b.modifier = load_modifier_0d(parser);

		context::input_binding_context().register_signal_binding(signal_table.insert(id), b);
	}

	static void load_mouse_button_binding(input::SignalTable& signal_table, const assets::Parser& parser, const std::string& id)
	{
		input::MouseButtonBinding b;
		if (!parser.optional(detail::Key::Button)(b.button))
			return;
		parser.optional(detail::Key::RequiredMods)(b.required_button_mods);
		parser.optional(detail::Key::ForbiddenMods)(b.forbidden_button_mods);
		b.modifier = load_modifier_0d(parser);

		context::input_binding_context().register_signal_binding(signal_table.insert(id), b);
	}

	static void load_gamepad_button_binding(input::SignalTable& signal_table, const assets::Parser& parser, const std::string& id)
	{
		int button;
		if (!parser.optional(detail::Key::Button)(button))
			return;
		input::GamepadButtonBinding b{ .button = (input::GamepadButton)button };
		b.modifier = load_modifier_0d(parser);

		context::input_binding_context().register_signal_binding(signal_table.insert(id), b);
	}

	static void load_gamepad_axis_1d_binding(input::SignalTable& signal_table, const assets::Parser& parser, const std::string& id)
	{
		int axis1d;
		if (!parser.optional(detail::Key::Axis1D)(axis1d))
			return;
		input::GamepadAxis1DBinding b{ .axis = (input::GamepadAxis1D)axis1d };
		parser.optional(detail::Key::Deadzone)(b.deadzone);
		b.modifier = load_modifier_1d(parser);

		context::input_binding_context().register_signal_binding(signal_table.insert(id), b);
	}

	static void load_gamepad_axis_2d_binding(input::SignalTable& signal_table, const assets::Parser& parser, const std::string& id)
	{
		int axis2d;
		if (!parser.optional(detail::Key::Axis2D)(axis2d))
			return;
		input::GamepadAxis2DBinding b{ .axis = (input::GamepadAxis2D)axis2d };
		parser.optional(detail::Key::Deadzone)(b.deadzone);
		b.modifier = load_modifier_2d(parser);

		context::input_binding_context().register_signal_binding(signal_table.insert(id), b);
	}

	static void load_cursor_pos_binding(input::SignalTable& signal_table, const assets::Parser& parser, const std::string& id)
	{
		input::CursorPosBinding b{};
		b.modifier = load_modifier_2d(parser);

		context::input_binding_context().register_signal_binding(signal_table.insert(id), b);
	}

	static void load_scroll_binding(input::SignalTable& signal_table, const assets::Parser& parser, const std::string& id)
	{
		input::ScrollBinding b{};
		b.modifier = load_modifier_2d(parser);

		context::input_binding_context().register_signal_binding(signal_table.insert(id), b);
	}

	static void load_signal(input::SignalTable& signal_table, TOMLNode node)
	{
		assets::Parser parser(node);
		const auto id = parser.required<std::string>(detail::Key::ID)();
		switch (parser.required<input::SignalBindingType>(detail::Key::Binding)())
		{
		case input::SignalBindingType::Key:
			load_key_binding(signal_table, parser, id);
			break;
		case input::SignalBindingType::MouseButton:
			load_mouse_button_binding(signal_table, parser, id);
			break;
		case input::SignalBindingType::GamepadButton:
			load_gamepad_button_binding(signal_table, parser, id);
			break;
		case input::SignalBindingType::GamepadAxis1D:
			load_gamepad_axis_1d_binding(signal_table, parser, id);
			break;
		case input::SignalBindingType::GamepadAxis2D:
			load_gamepad_axis_2d_binding(signal_table, parser, id);
			break;
		case input::SignalBindingType::CursorPos:
			load_cursor_pos_binding(signal_table, parser, id);
			break;
		case input::SignalBindingType::Scroll:
			load_scroll_binding(signal_table, parser, id);
			break;
		}
	}

	static void load_signal_routes(input::SignalRoutingTable& routing_table, TOMLNode node)
	{
		assets::Parser parser(node);
		const auto id = parser.required<std::string>(detail::Key::ID)();
		auto toml_signals = parser.required<TOMLArray>(detail::Key::Signals)();
		std::vector<std::string> signals;
		for (size_t i = 0; i < toml_signals->size(); ++i)
		{
			if (auto signal = toml_signals->get_as<std::string>(i))
				signals.push_back(signal->get());
			else
				_OLY_ENGINE_LOG_WARNING("CONTEXT") << "Input signal #" << i << " cannot be parsed as a string" << LOG.nl;
		}
		routing_table[id] = std::move(signals);
	}

	void InputController::load_signals(const detail::ResourcePath& file)
	{
		if (file.empty())
		{
			_OLY_ENGINE_LOG_ERROR("CONTEXT") << "Filename is empty" << LOG.nl;
			throw Error(ErrorCode::LoadAsset);
		}

		if (!detail::MetaSplitter::decode_meta(file).has_type(detail::Key::Meta_Signal))
		{
			_OLY_ENGINE_LOG_ERROR("CONTEXT") << "Meta fields do not contain signal type" << LOG.nl;
			throw Error(ErrorCode::LoadAsset);
		}

		auto toml = io::load_toml(file);
		assets::Parser parser(toml);

		if (auto signals = parser.optional<TOMLArray>(detail::Key::SignalArray)())
			signals->for_each([this](auto&& node) { load_signal(_signal_table, (TOMLNode)node); });

		if (auto mappings = parser.optional<TOMLArray>(detail::Key::RoutingArray)())
			mappings->for_each([this](auto&& node) { load_signal_routes(_signal_routing_table, (TOMLNode)node); });
	}
}
