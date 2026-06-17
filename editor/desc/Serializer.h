#pragma once

#include "core/Meta.h"
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
		bool Load(bool& obj, TOMLNode node) const
		{
			if (auto v = node.value<bool>())
			{
				obj = *v;
				return true;
			}
			else
				return false;
		}

		bool Dump(const bool obj) const
		{
			return obj;
		}
	};

	template<>
	struct Serializer<int>
	{
		bool Load(int& obj, TOMLNode node) const
		{
			if (auto v = node.value<int64_t>())
			{
				obj = static_cast<int>(*v);
				return true;
			}
			else
				return false;
		}

		int64_t Dump(const int obj) const
		{
			return static_cast<int64_t>(obj);
		}
	};

	template<>
	struct Serializer<unsigned int>
	{
		bool Load(unsigned int& obj, TOMLNode node) const
		{
			if (auto v = node.value<int64_t>())
			{
				obj = static_cast<unsigned int>(*v);
				return true;
			}
			else
				return false;
		}

		int64_t Dump(const unsigned int obj) const
		{
			return static_cast<int64_t>(obj);
		}
	};

	template<>
	struct Serializer<unsigned char>
	{
		bool Load(unsigned char& obj, TOMLNode node) const
		{
			if (auto v = node.value<int64_t>())
			{
				obj = static_cast<unsigned char>(*v);
				return true;
			}
			else
				return false;
		}

		int64_t Dump(const unsigned char obj) const
		{
			return static_cast<int64_t>(obj);
		}
	};

	template<Enum E>
	struct Serializer<E>
	{
		bool Load(E& obj, TOMLNode node) const
		{
			if (auto v = node.value<int64_t>())
			{
				obj = static_cast<E>(*v);
				return true;
			}
			else
				return false;
		}

		int64_t Dump(const E obj) const
		{
			return static_cast<int64_t>(obj);
		}
	};

	template<>
	struct Serializer<float>
	{
		bool Load(float& obj, TOMLNode node) const
		{
			if (auto v = node.value<double>())
			{
				obj = static_cast<float>(*v);
				return true;
			}
			else
				return false;
		}

		double Dump(const float obj) const
		{
			return static_cast<double>(obj);
		}
	};

	template<>
	struct Serializer<double>
	{
		bool Load(double& obj, TOMLNode node) const
		{
			if (auto v = node.value<double>())
			{
				obj = *v;
				return true;
			}
			else
				return false;
		}

		double Dump(const double obj) const
		{
			return obj;
		}
	};

	template<>
	struct Serializer<std::string>
	{
		bool Load(std::string& obj, TOMLNode node) const
		{
			if (auto v = node.value<std::string>())
			{
				obj = std::move(*v);
				return true;
			}
			else
				return false;
		}

		std::string Dump(const std::string& obj) const
		{
			return obj;
		}
	};

	template<typename T, glm::length_t L>
	struct Serializer<glm::vec<L, T>>
	{
		bool Load(glm::vec<L, T>& obj, TOMLNode node) const
		{
			if (auto arr = node.as_array())
			{
				bool fully_loaded = true;
				for (glm::length_t i = 0; i < glm::min(static_cast<glm::length_t>(arr->size()), L); ++i)
					fully_loaded &= Serializer<T>{}.Load(obj[i], TOMLNode(*arr->get(i)));
				return fully_loaded;
			}
			else
				return false;
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
		bool Load(std::array<T, N>& obj, TOMLNode node) const
		{
			if (auto arr = node.as_array())
			{
				bool fully_loaded = true;
				for (size_t i = 0; i < std::min(arr->size(), N); ++i)
					fully_loaded &= Serializer<T>{}.Load(obj[i], TOMLNode(*arr->get(i)));
				return fully_loaded;
			}
			else
				return false;
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
		bool Load(Color& obj, TOMLNode node) const
		{
			if (auto array = node.as_array())
			{
				bool fully_loaded = true;
				for (size_t i = 0; i < std::min(array->size(), Color::N); ++i)
				{
					if (auto v = array->get_as<double>(i))
						obj[i] = v->get();
					else
						fully_loaded = false;
				}
				return fully_loaded;
			}
			else
				return false;
		}

		toml::array Dump(const Color obj) const
		{
			toml::array arr;
			arr.reserve(Color::N);
			for (size_t i = 0; i < Color::N; ++i)
				arr.push_back(obj[i]);
			return arr;
		}
	};

	template<>
	struct Serializer<Rect>
	{
		bool Load(Rect& obj, TOMLNode node) const
		{
			if (auto array = node.as_array())
			{
				bool fully_loaded = true;
				for (size_t i = 0; i < std::min(array->size(), Rect::N); ++i)
				{
					if (auto v = array->get_as<double>(i))
						obj[i] = v->get();
					else
						fully_loaded = false;
				}
				return fully_loaded;
			}
			else
				return false;
		}

		toml::array Dump(const Rect obj) const
		{
			toml::array arr;
			arr.reserve(Rect::N);
			for (size_t i = 0; i < Color::N; ++i)
				arr.push_back(obj[i]);
			return arr;
		}
	};

	template<typename T>
	struct Serializer<std::vector<T>>
	{
		bool Load(std::vector<T>& obj, TOMLNode node) const
		{
			if (auto arr = node.as_array())
			{
				bool fully_loaded = true;
				for (size_t i = 0; i < arr->size(); ++i)
				{
					T el{};
					if (Serializer<T>{}.Load(el, TOMLNode(*arr->get(i))))
						obj.push_back(std::move(el));
					else
						fully_loaded = false;
				}
				return fully_loaded;
			}
			else
				return false;
		}

		toml::array Dump(const std::vector<T>& obj) const
		{
			toml::array arr;
			arr.reserve(obj.size());
			for (const T& el : obj)
				arr.push_back(Serializer<T>{}.Dump(el));
			return arr;
		}
	};
}
