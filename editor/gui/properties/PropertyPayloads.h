#pragma once

#include <imtk.hpp>

#include <optional>
#include <string>

namespace oly::editor::prop
{
	template<typename T>
	struct PropertyPayloadInterface
	{
		static imtk::prop::payload Dump(const T& value)
		{
			return imtk::prop::payload::pod(value);
		}

		static std::optional<T> Load(const imtk::prop::payload& payload)
		{
			if (auto data = payload.resolve<T>())
				return *data;
			else
				return std::nullopt;
		}
	};

	// TODO v9.3 put into separate common_interfaces file
	template<>
	struct PropertyPayloadInterface<std::string>
	{
		static imtk::prop::payload Dump(const std::string& value);
		static std::optional<std::string> Load(const imtk::prop::payload& payload);
	};
}
