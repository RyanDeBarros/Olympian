#pragma once

#include "external/GLM.h"
#include "core/base/Errors.h"
#include "core/types/DeferredFalse.h"

namespace oly::input
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

	typedef std::unordered_map<std::string, std::vector<std::string>> SignalMappingTable;

	enum class Phase
	{
		STARTED,
		COMPLETED,
		ONGOING
	};

	struct Signal
	{
		Phase phase;

		enum class Type
		{
			BOOL,
			AXIS1D,
			AXIS2D,
			AXIS3D
		};

		enum Source
		{
			KEYBOARD = -2,
			MOUSE = -1,
			JOYSTICK_BASE = 0,
		};

	private:
		Type type;
		Source source;

		union
		{
			bool bool_value;
			float axis_1d_value;
			glm::vec2 axis_2d_value;
			glm::vec3 axis_3d_value;
		};

	public:
		Signal(Phase phase, bool v, Source source) : phase(phase), type(Type::BOOL), bool_value(v), source(source) {}
		Signal(Phase phase, float v, Source source) : phase(phase), type(Type::AXIS1D), axis_1d_value(v), source(source) {}
		Signal(Phase phase, glm::vec2 v, Source source) : phase(phase), type(Type::AXIS2D), axis_2d_value(v), source(source) {}
		Signal(Phase phase, glm::vec3 v, Source source) : phase(phase), type(Type::AXIS3D), axis_3d_value(v), source(source) {}

		template<typename T>
		T get() const
		{
			static_assert(deferred_false<T>, "oly::input::Signal::get<T>() does not support the invoked type.");
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

		template<>
		glm::vec3 get<glm::vec3>() const
		{
			if (type != Type::AXIS3D)
				throw Error(ErrorCode::INCOMPATIBLE_SIGNAL_TYPE);
			return axis_3d_value;
		}

		bool from_controller() const { return source >= Source::JOYSTICK_BASE; }
		int controller_id() const
		{
			if (source < Source::JOYSTICK_BASE)
				throw Error(ErrorCode::INVALID_CONTROLLER_ID);
			return source;
		}
	};
}
