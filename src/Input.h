#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <variant>
#include <unordered_map>
#include <string>

#include "Events.h"

namespace oly
{
	namespace input
	{
		struct CharEventData
		{
			unsigned int codepoint;
		};

		struct CharModsEventData
		{
			unsigned int codepoint;
			int mods;
		};

		struct CursorEnterEventData
		{
			bool entered;
		};

		struct CursorPosEventData
		{
			double x, y;
		};

		struct PathDropEventData
		{
			int count;
			const char** paths;
		};

		struct FramebufferResizeEventData
		{
			int w, h;
		};

		struct KeyEventData
		{
			int key;
			int scancode;
			int action;
			int mods;
		};

		struct MouseButtonEventData
		{
			int button;
			int action;
			int mods;
		};

		struct ScrollEventData
		{
			double xoff, yoff;
		};

		struct WindowCloseEventData
		{
		};

		struct WindowContentScaleEventData
		{
			float x, y;
		};

		struct WindowFocusEventData
		{
			bool focused;
		};

		struct WindowIconifyEventData
		{
			bool iconified;
		};

		struct WindowMaximizeEventData
		{
			bool maximized;
		};

		struct WindowPosEventData
		{
			int x, y;
		};

		struct WindowRefreshEventData
		{
		};

		struct WindowResizeEventData
		{
			int w, h;
		};

		extern void init_handlers(GLFWwindow* w);

		struct GamepadEventData
		{
			int jevent;
		};

		class Gamepad
		{
			GLFWgamepadstate g;
			int c;

		public:
			EventHandler<GamepadEventData> handler;

			Gamepad(int controller) : c(controller) { glfwGetGamepadState(controller, &g); }

			int controller() const { return c; }
			bool connected() const { return glfwJoystickPresent(c); }
			bool has_mapping() const { return glfwJoystickIsGamepad(c); }
			int button_state(int button) const { return g.buttons[button]; }
			float axis_state(int axis) const { return g.axes[axis]; }
			int dpad_state(unsigned int dpad = 0) const { int count; const unsigned char* hats = glfwGetJoystickHats(c, &count); return dpad < (unsigned int)count ? hats[dpad] : 0; }
			const char* name() const { return glfwGetJoystickName(c); }
			const char* identifier() const { return glfwGetJoystickGUID(c); }
			void set_handler() { glfwSetJoystickUserPointer(c, this); }
		};

		extern void init_joystick_handler();

		typedef unsigned int SignalID;

		class SignalTable
		{
			SignalID next = 1;
			std::unordered_map<std::string, SignalID> table;

		public:
			SignalID get(const std::string& name)
			{
				auto it = table.find(name);
				if (it != table.end())
					return it->second;
				return table.emplace(name, next++).first->second;
			}
		};

		enum class Phase
		{
			STARTED,
			COMPLETED,
			ONGOING
		};

		struct InputSignal0D
		{
			Phase phase;
		};

		struct InputSignal1D
		{
			Phase phase;
			float v;
		};

		struct InputSignal2D
		{
			Phase phase;
			glm::vec2 v;
		};

		struct InputController
		{
			virtual ~InputController() = default;

			typedef bool(InputController::* Handler0D)(InputSignal0D);
			typedef bool(InputController::* Handler1D)(InputSignal1D);
			typedef bool(InputController::* Handler2D)(InputSignal2D);
		};

		struct KeyBinding
		{
			int key;
			int required_mods = 0;
			int forbidden_mods = 0;

			bool matches(int key, int mods) const
			{
				if (key != this->key)
					return false;
				if ((mods & required_mods) != required_mods)
					return false;
				if ((mods & forbidden_mods) != 0)
					return false;
				return true;
			}

			bool operator==(const KeyBinding&) const = default;
		};

		struct MouseButtonBinding
		{
			int button;
			int required_mods = 0;
			int forbidden_mods = 0;

			bool matches(int button, int mods) const
			{
				if (button != this->button)
					return false;
				if ((mods & required_mods) != required_mods)
					return false;
				if ((mods & forbidden_mods) != 0)
					return false;
				return true;
			}

			bool operator==(const MouseButtonBinding&) const = default;
		};

		struct CursorPosBinding
		{
			bool operator==(const CursorPosBinding&) const = default;
		};

		class BindingContext : public EventHandler<KeyEventData>, public EventHandler<MouseButtonEventData>, public EventHandler<CursorPosEventData>
		{
			std::vector<std::pair<SignalID, KeyBinding>> key_bindings;
			std::vector<std::pair<SignalID, MouseButtonBinding>> mb_bindings;
			std::vector<std::pair<SignalID, CursorPosBinding>> cpos_bindings;

			struct
			{
				double callback_time = 0.0;
				bool cursor_moving = false;
			} cpos_poll;

			struct InputControllerRef0D
			{
				InputController::Handler0D handler;
				InputController* controller;
			};
			std::unordered_map<SignalID, InputControllerRef0D> handler_0d_map;

			struct InputControllerRef1D
			{
				InputController::Handler1D handler;
				InputController* controller;
			};
			std::unordered_map<SignalID, InputControllerRef1D> handler_1d_map;

			struct InputControllerRef2D
			{
				InputController::Handler2D handler;
				InputController* controller;
			};
			std::unordered_map<SignalID, InputControllerRef2D> handler_2d_map;

			friend class Context;

			void attach_key(EventHandler<KeyEventData>* parent) { EventHandler<KeyEventData>::attach(parent); }
			void attach_mouse_button(EventHandler<MouseButtonEventData>* parent) { EventHandler<MouseButtonEventData>::attach(parent); }
			void attach_cursor_pos(EventHandler<CursorPosEventData>* parent) { EventHandler<CursorPosEventData>::attach(parent); }
			void detach_key() { EventHandler<KeyEventData>::detach(); }
			void detach_mouse_button() { EventHandler<MouseButtonEventData>::detach(); }
			void detach_cusor_pos() { EventHandler<CursorPosEventData>::detach(); }

#define REG_SIGNAL(Binding, vector) void register_signal(SignalID signal, Binding binding) { vector.push_back({ signal, binding }); }\
									void unregister_signal(SignalID signal, Binding binding)\
											{ vector.erase(std::find(vector.begin(), vector.end(), std::make_pair(signal, binding))); }

		public:
			REG_SIGNAL(KeyBinding, key_bindings)
			REG_SIGNAL(MouseButtonBinding, mb_bindings)
			REG_SIGNAL(CursorPosBinding, cpos_bindings)

			void bind(SignalID signal, InputController::Handler0D handler, InputController* controller) { handler_0d_map[signal] = { handler, controller }; }
			void unbind(SignalID signal, InputController::Handler0D handler, InputController* controller)
			{
				auto it = handler_0d_map.find(signal);
				if (it != handler_0d_map.end() && it->second.handler == handler && it->second.controller == controller)
					handler_0d_map.erase(it);
			}

			void bind(SignalID signal, InputController::Handler1D handler, InputController* controller) { handler_1d_map[signal] = { handler, controller }; }
			void unbind(SignalID signal, InputController::Handler1D handler, InputController* controller)
			{
				auto it = handler_1d_map.find(signal);
				if (it != handler_1d_map.end() && it->second.handler == handler && it->second.controller == controller)
					handler_1d_map.erase(it);
			}

			void bind(SignalID signal, InputController::Handler2D handler, InputController* controller) { handler_2d_map[signal] = { handler, controller }; }
			void unbind(SignalID signal, InputController::Handler2D handler, InputController* controller)
			{
				auto it = handler_2d_map.find(signal);
				if (it != handler_2d_map.end() && it->second.handler == handler && it->second.controller == controller)
					handler_2d_map.erase(it);
			}

			// call poll(window) after glfwPollEvents()
			void poll(GLFWwindow* window);

		private:
			bool get_phase(int action, Phase& phase) const;
			bool dispatch(SignalID id, InputSignal0D signal) const;
			bool dispatch(SignalID id, InputSignal2D signal) const;

		public:
			virtual bool consume(const KeyEventData& data) override;
			virtual bool consume(const MouseButtonEventData& data) override;
			virtual bool consume(const CursorPosEventData& data) override;
		};
	}
}
