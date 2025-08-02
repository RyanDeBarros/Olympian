#pragma once

#include <memory>

#include "external/TOML.h"
#include "core/platform/Window.h"
#include "core/platform/Gamepad.h"
#include "core/platform/BindingContext.h"

namespace oly::platform
{
	struct PlatformSetup
	{
		WindowHint window_hint;
		int window_width, window_height;
		std::string window_title;

		int num_gamepads;

		PlatformSetup() = default;
		PlatformSetup(const TOMLNode& node);

		glm::ivec2 window_size() const { return { window_width, window_height }; }
	};

	class Platform;

	namespace internal
	{
		extern std::unique_ptr<Platform> create_platform(const PlatformSetup&);
	}

	class Platform
	{
		Window _window;
		FixedVector<Gamepad> _gamepads;
		int _num_gamepads = 0;
		input::SignalTable _signal_table;
		input::SignalMappingTable _signal_mapping_table;
		InputBindingContext _binding_context;

		friend std::unique_ptr<Platform> internal::create_platform(const PlatformSetup&);
		Platform(const PlatformSetup& setup);

	public:
		Platform(const Platform&) = delete;
		Platform(Platform&& platform) noexcept = default;

		GLenum per_frame_clear_mask = GL_COLOR_BUFFER_BIT;
		bool frame();

		const Window& window() const { return _window; }
		Window& window() { return _window; }
		const Gamepad& gamepad(int i = 0) const { return _gamepads[i]; }
		Gamepad& gamepad(int i = 0) { return _gamepads[i]; }
		const input::SignalTable& signal_table() const { return _signal_table; }
		input::SignalTable& signal_table() { return _signal_table; }
		const InputBindingContext& binding_context() const { return _binding_context; }
		InputBindingContext& binding_context() { return _binding_context; }

		void assign_signal_mapping(const std::string& mapping_name, std::vector<std::string>&& signal_names)
		{
			_signal_mapping_table[mapping_name] = std::move(signal_names);
		}

		void unassign_signal_mapping(const std::string& mapping_name)
		{
			_signal_mapping_table.erase(mapping_name);
		}

		template<std::derived_from<InputController> Controller>
		void bind_signal(const std::string& signal, bool(Controller::* handler)(input::Signal), const SoftReference<Controller>& controller)
		{
			_binding_context.bind(_signal_table.get(signal), static_cast<InputController::Handler>(handler), controller);
		}

		template<std::derived_from<InputController> Controller>
		void bind_signal(const std::string& signal, bool(Controller::* handler)(input::Signal) const, const ConstSoftReference<Controller>& controller)
		{
			_binding_context.bind(_signal_table.get(signal), static_cast<InputController::ConstHandler>(handler), controller);
		}

		template<std::derived_from<InputController> Controller>
		void bind_signal(const std::string& signal, bool(Controller::* handler)(input::Signal), Controller& controller)
		{
			_binding_context.bind(_signal_table.get(signal), static_cast<InputController::Handler>(handler), controller.ref());
		}

		template<std::derived_from<InputController> Controller>
		void bind_signal(const std::string& signal, bool(Controller::* handler)(input::Signal) const, const Controller& controller)
		{
			_binding_context.bind(_signal_table.get(signal), static_cast<InputController::ConstHandler>(handler), controller.cref());
		}

		template<std::derived_from<InputController> Controller>
		void unbind_signal(const std::string& signal, bool(Controller::* handler)(input::Signal), const SoftReference<Controller>& controller)
		{
			_binding_context.unbind(_signal_table.get(signal), static_cast<InputController::Handler>(handler), controller);
		}

		template<std::derived_from<InputController> Controller>
		void unbind_signal(const std::string& signal, bool(Controller::* handler)(input::Signal) const, const ConstSoftReference<Controller>& controller)
		{
			_binding_context.unbind(_signal_table.get(signal), static_cast<InputController::ConstHandler>(handler), controller);
		}

		template<std::derived_from<InputController> Controller>
		void unbind_signal(const std::string& signal, bool(Controller::* handler)(input::Signal), Controller& controller)
		{
			_binding_context.unbind(_signal_table.get(signal), static_cast<InputController::Handler>(handler), controller.ref());
		}

		template<std::derived_from<InputController> Controller>
		void unbind_signal(const std::string& signal, bool(Controller::* handler)(input::Signal) const, const Controller& controller)
		{
			_binding_context.unbind(_signal_table.get(signal), static_cast<InputController::ConstHandler>(handler), controller.cref());
		}

		void unbind_signal(const std::string& signal)
		{
			_binding_context.unbind(_signal_table.get(signal));
		}

		template<std::derived_from<InputController> Controller>
		void bind_signal_mapping(const std::string& mapping, bool(Controller::* handler)(input::Signal), const SoftReference<Controller>& controller)
		{
			const std::vector<std::string>& signals = _signal_mapping_table[mapping];
			for (const std::string& signal : signals)
				bind_signal(signal, handler, controller);
		}

		template<std::derived_from<InputController> Controller>
		void bind_signal_mapping(const std::string& mapping, bool(Controller::* handler)(input::Signal) const, const ConstSoftReference<Controller>& controller)
		{
			const std::vector<std::string>& signals = _signal_mapping_table[mapping];
			for (const std::string& signal : signals)
				bind_signal(signal, handler, controller);
		}

		template<std::derived_from<InputController> Controller>
		void bind_signal_mapping(const std::string& mapping, bool(Controller::* handler)(input::Signal), Controller& controller)
		{
			const std::vector<std::string>& signals = _signal_mapping_table[mapping];
			for (const std::string& signal : signals)
				bind_signal(signal, handler, controller);
		}

		template<std::derived_from<InputController> Controller>
		void bind_signal_mapping(const std::string& mapping, bool(Controller::* handler)(input::Signal) const, const Controller& controller)
		{
			const std::vector<std::string>& signals = _signal_mapping_table[mapping];
			for (const std::string& signal : signals)
				bind_signal(signal, handler, controller);
		}

		template<std::derived_from<InputController> Controller>
		void unbind_signal_mapping(const std::string& mapping, bool(Controller::* handler)(input::Signal), const SoftReference<Controller>& controller)
		{
			const std::vector<std::string>& signals = _signal_mapping_table[mapping];
			for (const std::string& signal : signals)
				unbind_signal(signal, handler, controller);
		}

		template<std::derived_from<InputController> Controller>
		void unbind_signal_mapping(const std::string& mapping, bool(Controller::* handler)(input::Signal) const, const ConstSoftReference<Controller>& controller)
		{
			const std::vector<std::string>& signals = _signal_mapping_table[mapping];
			for (const std::string& signal : signals)
				unbind_signal(signal, handler, controller);
		}

		template<std::derived_from<InputController> Controller>
		void unbind_signal_mapping(const std::string& mapping, bool(Controller::* handler)(input::Signal), Controller& controller)
		{
			const std::vector<std::string>& signals = _signal_mapping_table[mapping];
			for (const std::string& signal : signals)
				unbind_signal(signal, handler, controller);
		}

		template<std::derived_from<InputController> Controller>
		void unbind_signal_mapping(const std::string& mapping, bool(Controller::* handler)(input::Signal) const, const Controller& controller)
		{
			const std::vector<std::string>& signals = _signal_mapping_table[mapping];
			for (const std::string& signal : signals)
				unbind_signal(signal, handler, controller);
		}

		void unbind_signal_mapping(const std::string& mapping)
		{
			const std::vector<std::string>& signals = _signal_mapping_table[mapping];
			for (const std::string& signal : signals)
				unbind_signal(signal);
		}
	};
}
