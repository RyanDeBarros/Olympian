#pragma once

#include "external/GLM.h"
#include "core/base/Errors.h"
#include "core/types/DeferredFalse.h"
#include "core/util/StringParam.h"

namespace oly::input
{
	typedef unsigned int SignalID;

	class SignalTable
	{
		SignalID next = 1;
		std::unordered_map<std::string, SignalID, StringParamHeteroHash, StringParamHeteroEqual> table;

	public:
		SignalID get(const StringParam& name)
		{
			auto it = table.find(name);
			if (it != table.end())
				return it->second;
			return table.emplace(name.transfer(), next++).first->second;
		}
	};

	typedef std::unordered_map<std::string, std::vector<std::string>, StringParamHeteroHash, StringParamHeteroEqual> SignalMappingTable;

	enum class Phase
	{
		Started,
		Completed,
		Ongoing
	};

	struct Signal
	{
		Phase phase;

		enum class Type
		{
			Boolean,
			Axis1D,
			Axis2D,
			Axis3D
		};

		enum Source
		{
			Keyboard = -2,
			Mouse = -1,
			JoystickBase = 0,
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
		Signal(Phase phase, bool v, Source source) : phase(phase), type(Type::Boolean), bool_value(v), source(source) {}
		Signal(Phase phase, float v, Source source) : phase(phase), type(Type::Axis1D), axis_1d_value(v), source(source) {}
		Signal(Phase phase, glm::vec2 v, Source source) : phase(phase), type(Type::Axis2D), axis_2d_value(v), source(source) {}
		Signal(Phase phase, glm::vec3 v, Source source) : phase(phase), type(Type::Axis3D), axis_3d_value(v), source(source) {}

		template<typename T>
		T get() const
		{
			static_assert(deferred_false<T>, "oly::input::Signal::get<T>() does not support the invoked type.");
		}

		template<>
		bool get<bool>() const
		{
			if (type != Type::Boolean)
				throw Error(ErrorCode::IncompatibleSignalType);
			return bool_value;
		}

		template<>
		float get<float>() const
		{
			if (type != Type::Axis1D)
				throw Error(ErrorCode::IncompatibleSignalType);
			return axis_1d_value;
		}

		template<>
		glm::vec2 get<glm::vec2>() const
		{
			if (type != Type::Axis2D)
				throw Error(ErrorCode::IncompatibleSignalType);
			return axis_2d_value;
		}

		template<>
		glm::vec3 get<glm::vec3>() const
		{
			if (type != Type::Axis3D)
				throw Error(ErrorCode::IncompatibleSignalType);
			return axis_3d_value;
		}

		bool from_controller() const { return source >= Source::JoystickBase; }
		int controller_id() const
		{
			if (source < Source::JoystickBase)
				throw Error(ErrorCode::InvalidControllerID);
			return source;
		}
	};
}
