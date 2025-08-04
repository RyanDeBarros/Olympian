#pragma once

#include "core/platform/Platform.h"
#include "core/platform/WindowResize.h"
#include "core/platform/BindingContext.h"

namespace oly::context
{
	namespace internal
	{
		extern void init_platform(const TOMLNode&);
		extern void init_viewport(const TOMLNode&);
		extern void terminate_platform();
		extern bool frame_platform();
	}

	extern platform::Platform& get_platform();
	extern void set_window_resize_mode(bool boxed = true, bool stretch = true);
	extern const platform::WRViewport& get_wr_viewport();
	extern platform::WRDrawer& get_wr_drawer();
	extern void set_standard_viewport();

	extern glm::vec2 get_cursor_screen_pos();
	extern glm::vec2 get_initial_window_size();
	extern glm::vec2 get_view_stretch();
	extern glm::vec2 get_cursor_view_pos();

	extern input::internal::InputBindingContext& input_binding_context();
	extern input::SignalTable& signal_table();
	extern input::SignalMappingTable& signal_mapping_table();

	extern void assign_signal_mapping(const std::string& mapping_name, std::vector<std::string>&& signal_names);
	extern void unassign_signal_mapping(const std::string& mapping_name);

	template<std::derived_from<InputController> Controller>
	inline void bind_signal(const std::string& signal, bool(Controller::* handler)(input::Signal), const SoftReference<Controller>& controller)
	{
		input_binding_context().bind(signal_table().get(signal), static_cast<InputController::Handler>(handler), controller);
	}

	template<std::derived_from<InputController> Controller>
	inline void bind_signal(const std::string& signal, bool(Controller::* handler)(input::Signal) const, const ConstSoftReference<Controller>& controller)
	{
		input_binding_context().bind(signal_table().get(signal), static_cast<InputController::ConstHandler>(handler), controller);
	}

	template<std::derived_from<InputController> Controller>
	inline void bind_signal(const std::string& signal, bool(Controller::* handler)(input::Signal), Controller& controller)
	{
		input_binding_context().bind(signal_table().get(signal), static_cast<InputController::Handler>(handler), controller.ref());
	}

	template<std::derived_from<InputController> Controller>
	inline void bind_signal(const std::string& signal, bool(Controller::* handler)(input::Signal) const, const Controller& controller)
	{
		input_binding_context().bind(signal_table().get(signal), static_cast<InputController::ConstHandler>(handler), controller.cref());
	}

	template<std::derived_from<InputController> Controller>
	inline void unbind_signal(const std::string& signal, bool(Controller::* handler)(input::Signal), const SoftReference<Controller>& controller)
	{
		input_binding_context().unbind(signal_table().get(signal), static_cast<InputController::Handler>(handler), controller);
	}

	template<std::derived_from<InputController> Controller>
	inline void unbind_signal(const std::string& signal, bool(Controller::* handler)(input::Signal) const, const ConstSoftReference<Controller>& controller)
	{
		input_binding_context().unbind(signal_table().get(signal), static_cast<InputController::ConstHandler>(handler), controller);
	}

	template<std::derived_from<InputController> Controller>
	inline void unbind_signal(const std::string& signal, bool(Controller::* handler)(input::Signal), Controller& controller)
	{
		input_binding_context().unbind(signal_table().get(signal), static_cast<InputController::Handler>(handler), controller.ref());
	}

	template<std::derived_from<InputController> Controller>
	inline void unbind_signal(const std::string& signal, bool(Controller::* handler)(input::Signal) const, const Controller& controller)
	{
		input_binding_context().unbind(signal_table().get(signal), static_cast<InputController::ConstHandler>(handler), controller.cref());
	}

	template<std::derived_from<InputController> Controller>
	inline void bind_signal_mapping(const std::string& mapping, bool(Controller::* handler)(input::Signal), const SoftReference<Controller>& controller)
	{
		const std::vector<std::string>& signals = signal_mapping_table()[mapping];
		for (const std::string& signal : signals)
			bind_signal(signal, handler, controller);
	}

	template<std::derived_from<InputController> Controller>
	inline void bind_signal_mapping(const std::string& mapping, bool(Controller::* handler)(input::Signal) const, const ConstSoftReference<Controller>& controller)
	{
		const std::vector<std::string>& signals = signal_mapping_table()[mapping];
		for (const std::string& signal : signals)
			bind_signal(signal, handler, controller);
	}

	template<std::derived_from<InputController> Controller>
	inline void bind_signal_mapping(const std::string& mapping, bool(Controller::* handler)(input::Signal), Controller& controller)
	{
		const std::vector<std::string>& signals = signal_mapping_table()[mapping];
		for (const std::string& signal : signals)
			bind_signal(signal, handler, controller);
	}

	template<std::derived_from<InputController> Controller>
	inline void bind_signal_mapping(const std::string& mapping, bool(Controller::* handler)(input::Signal) const, const Controller& controller)
	{
		const std::vector<std::string>& signals = signal_mapping_table()[mapping];
		for (const std::string& signal : signals)
			bind_signal(signal, handler, controller);
	}

	template<std::derived_from<InputController> Controller>
	inline void unbind_signal_mapping(const std::string& mapping, bool(Controller::* handler)(input::Signal), const SoftReference<Controller>& controller)
	{
		const std::vector<std::string>& signals = signal_mapping_table()[mapping];
		for (const std::string& signal : signals)
			unbind_signal(signal, handler, controller);
	}

	template<std::derived_from<InputController> Controller>
	inline void unbind_signal_mapping(const std::string& mapping, bool(Controller::* handler)(input::Signal) const, const ConstSoftReference<Controller>& controller)
	{
		const std::vector<std::string>& signals = signal_mapping_table()[mapping];
		for (const std::string& signal : signals)
			unbind_signal(signal, handler, controller);
	}

	template<std::derived_from<InputController> Controller>
	inline void unbind_signal_mapping(const std::string& mapping, bool(Controller::* handler)(input::Signal), Controller& controller)
	{
		const std::vector<std::string>& signals = signal_mapping_table()[mapping];
		for (const std::string& signal : signals)
			unbind_signal(signal, handler, controller);
	}

	template<std::derived_from<InputController> Controller>
	inline void unbind_signal_mapping(const std::string& mapping, bool(Controller::* handler)(input::Signal) const, const Controller& controller)
	{
		const std::vector<std::string>& signals = signal_mapping_table()[mapping];
		for (const std::string& signal : signals)
			unbind_signal(signal, handler, controller);
	}
}
