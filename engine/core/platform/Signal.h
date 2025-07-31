#pragma once

#include "core/base/Errors.h"
#include "external/GLM.h"

namespace oly
{
	namespace platform { class InputBindingContext; }

	namespace input
	{
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

		struct Signal
		{
			Phase phase;

		private:
			enum class Type
			{
				BOOL,
				AXIS1D,
				AXIS2D
			} type;

			union
			{
				bool bool_value;
				float axis_1d_value;
				glm::vec2 axis_2d_value;
			};

			enum Source
			{
				KEYBOARD = -2,
				MOUSE = -1,
				JOYSTICK = 0,
			} source;

			friend class platform::InputBindingContext;
			Signal(Phase phase, bool v, Source source) : phase(phase), type(Type::BOOL), bool_value(v), source(source) {}
			Signal(Phase phase, float v, Source source) : phase(phase), type(Type::AXIS1D), axis_1d_value(v), source(source) {}
			Signal(Phase phase, glm::vec2 v, Source source) : phase(phase), type(Type::AXIS2D), axis_2d_value(v), source(source) {}

		public:
			template<typename T>
			T get() const
			{
				static_assert(false, "Signal::get<T>() does not support the invoked type.");
			}

			template<>
			bool get<bool>() const
			{
				if (type != Type::BOOL)
					throw Error(ErrorCode::INCOMPATIBLE_SIGNAL_TYPE);
				return bool_value;
			}

			template<>
			float get<float>() const
			{
				if (type != Type::AXIS1D)
					throw Error(ErrorCode::INCOMPATIBLE_SIGNAL_TYPE);
				return axis_1d_value;
			}

			template<>
			glm::vec2 get<glm::vec2>() const
			{
				if (type != Type::AXIS2D)
					throw Error(ErrorCode::INCOMPATIBLE_SIGNAL_TYPE);
				return axis_2d_value;
			}

			bool from_controller() const { return source >= Source::JOYSTICK; }
			int controller_id() const
			{
				if (source < Source::JOYSTICK)
					throw Error(ErrorCode::INVALID_CONTROLLER_ID);
				return source;
			}
		};
	}
}
