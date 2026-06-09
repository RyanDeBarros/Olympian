#pragma once

#include "core/Types.h"

#include "desc/OptionalPrimitive.h"

#include "external/TOML.h"
#include "external/GLM.h"

namespace oly::editor
{
	template<typename T>
	struct Serializer;

	template<>
	struct Serializer<bool>
	{
		void Load(bool& obj, TOMLNode node) const
		{
			if (auto v = node.value<bool>())
				obj = *v;
		}

		bool Dump(const bool obj) const
		{
			return obj;
		}
	};

	template<>
	struct Serializer<int>
	{
		void Load(int& obj, TOMLNode node) const
		{
			if (auto v = node.value<int64_t>())
				obj = static_cast<int>(*v);
		}

		int64_t Dump(const int obj) const
		{
			return static_cast<int64_t>(obj);
		}
	};

	template<typename T>
	concept Enum = std::is_enum_v<T>;

	template<Enum E>
	struct Serializer<E>
	{
		void Load(E& obj, TOMLNode node) const
		{
			if (auto v = node.value<int64_t>())
				obj = static_cast<E>(*v);
		}

		int64_t Dump(const E obj) const
		{
			return static_cast<int64_t>(obj);
		}
	};

	template<>
	struct Serializer<float>
	{
		void Load(float& obj, TOMLNode node) const
		{
			if (auto v = node.value<double>())
				obj = static_cast<float>(*v);
		}

		double Dump(const float obj) const
		{
			return static_cast<double>(obj);
		}
	};

	template<>
	struct Serializer<double>
	{
		void Load(double& obj, TOMLNode node) const
		{
			if (auto v = node.value<double>())
				obj = *v;
		}

		double Dump(const double obj) const
		{
			return obj;
		}
	};

	template<>
	struct Serializer<std::string>
	{
		void Load(std::string& obj, TOMLNode node) const
		{
			if (auto v = node.value<std::string>())
				obj = std::move(*v);
		}

		std::string Dump(const std::string& obj) const
		{
			return obj;
		}
	};

	template<typename T, glm::length_t L>
	struct Serializer<glm::vec<L, T>>
	{
		void Load(glm::vec<L, T>& obj, TOMLNode node) const
		{
			if (auto arr = node.as_array())
			{
				for (glm::length_t i = 0; i < glm::min(arr->size(), L); ++i)
					Serializer<T>{}.Load(obj[i], *arr->get(i));
			}
		}

		toml::array Dump(const glm::vec<L, T> obj) const
		{
			toml::array arr;
			arr.reserve(L);
			for (glm::length_t i = 0; i < L; ++i)
				arr.push_back(Serializer<T>{}.Dump(obj[i]));
			return arr;
		}
	};

	template<typename T, size_t N>
	struct Serializer<std::array<T, N>>
	{
		void Load(std::array<T, N>& obj, TOMLNode node) const
		{
			if (auto arr = node.as_array())
			{
				for (size_t i = 0; i < std::min(arr->size(), N); ++i)
					Serializer<T>{}.Load(obj[i], *arr->get(i));
			}
		}

		toml::array Dump(const std::array<T, N>& obj) const
		{
			toml::array arr;
			arr.reserve(N);
			for (size_t i = 0; i < N; ++i)
				arr.push_back(Serializer<T>{}.Dump(obj[i]));
			return arr;
		}
	};

	template<>
	struct Serializer<Color>
	{
		void Load(Color& obj, TOMLNode node) const
		{
			if (auto array = node.as_array())
			{
				for (size_t i = 0; i < std::min(array->size(), Color::N); ++i)
				{
					if (auto v = array->get_as<double>(i))
						obj[i] = v->get();
				}
			}
		}

		toml::array Dump(const Color obj) const
		{
			toml::array arr;
			arr.reserve(Color::N);
			arr.push_back(obj.r);
			arr.push_back(obj.g);
			arr.push_back(obj.b);
			arr.push_back(obj.a);
			return arr;
		}
	};
}
