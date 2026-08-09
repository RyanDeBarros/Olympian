#pragma once

#include "core/Meta.h"
#include "core/Types.h"

#include "desc/OptionalPrimitive.h"

#include "external/TOML.h"
#include "external/GLM.h"

#include "assets/ResourcePath.h"

#include <set>

// TODO v9.3 move serializer (all of desc - without impl/) to imtk

namespace oly::editor
{
	template<typename T>
	struct Serializer;

	template<>
	struct Serializer<bool>
	{
		bool load(bool& obj, TOMLNode node) const
		{
			if (auto v = node.value<bool>())
			{
				obj = *v;
				return true;
			}
			else
				return false;
		}

		bool dump(const bool obj) const
		{
			return obj;
		}
	};

	template<>
	struct Serializer<int>
	{
		bool load(int& obj, TOMLNode node) const
		{
			if (auto v = node.value<int64_t>())
			{
				obj = static_cast<int>(*v);
				return true;
			}
			else
				return false;
		}

		int64_t dump(const int obj) const
		{
			return static_cast<int64_t>(obj);
		}
	};

	template<>
	struct Serializer<int64_t>
	{
		bool load(int64_t& obj, TOMLNode node) const
		{
			if (auto v = node.value<int64_t>())
			{
				obj = *v;
				return true;
			}
			else
				return false;
		}

		int64_t dump(const int64_t obj) const
		{
			return obj;
		}
	};

	template<>
	struct Serializer<unsigned int>
	{
		bool load(unsigned int& obj, TOMLNode node) const
		{
			if (auto v = node.value<int64_t>())
			{
				obj = static_cast<unsigned int>(*v);
				return true;
			}
			else
				return false;
		}

		int64_t dump(const unsigned int obj) const
		{
			return static_cast<int64_t>(obj);
		}
	};

	template<>
	struct Serializer<unsigned char>
	{
		bool load(unsigned char& obj, TOMLNode node) const
		{
			if (auto v = node.value<int64_t>())
			{
				obj = static_cast<unsigned char>(*v);
				return true;
			}
			else
				return false;
		}

		int64_t dump(const unsigned char obj) const
		{
			return static_cast<int64_t>(obj);
		}
	};

	template<Enum E>
	struct Serializer<E>
	{
		bool load(E& obj, TOMLNode node) const
		{
			if (auto v = node.value<int64_t>())
			{
				obj = static_cast<E>(*v);
				return true;
			}
			else
				return false;
		}

		int64_t dump(const E obj) const
		{
			return static_cast<int64_t>(obj);
		}
	};

	template<>
	struct Serializer<float>
	{
		bool load(float& obj, TOMLNode node) const
		{
			if (auto v = node.value<double>())
			{
				obj = static_cast<float>(*v);
				return true;
			}
			else
				return false;
		}

		double dump(const float obj) const
		{
			return static_cast<double>(obj);
		}
	};

	template<>
	struct Serializer<double>
	{
		bool load(double& obj, TOMLNode node) const
		{
			if (auto v = node.value<double>())
			{
				obj = *v;
				return true;
			}
			else
				return false;
		}

		double dump(const double obj) const
		{
			return obj;
		}
	};

	template<>
	struct Serializer<std::string>
	{
		bool load(std::string& obj, TOMLNode node) const
		{
			if (auto v = node.value<std::string>())
			{
				obj = std::move(*v);
				return true;
			}
			else
				return false;
		}

		std::string dump(const std::string& obj) const
		{
			return obj;
		}
	};

	template<typename T, glm::length_t L>
	struct Serializer<glm::vec<L, T>>
	{
		bool load(glm::vec<L, T>& obj, TOMLNode node) const
		{
			if (auto arr = node.as_array())
			{
				bool fully_loaded = true;
				for (glm::length_t i = 0; i < glm::min(static_cast<glm::length_t>(arr->size()), L); ++i)
					fully_loaded &= Serializer<T>{}.load(obj[i], TOMLNode(*arr->get(i)));
				return fully_loaded;
			}
			else
				return false;
		}

		toml::array dump(const glm::vec<L, T> obj) const
		{
			toml::array arr;
			arr.reserve(L);
			for (glm::length_t i = 0; i < L; ++i)
				arr.push_back(Serializer<T>{}.dump(obj[i]));
			return arr;
		}
	};

	template<typename T, size_t N>
	struct Serializer<std::array<T, N>>
	{
		bool load(std::array<T, N>& obj, TOMLNode node) const
		{
			if (auto arr = node.as_array())
			{
				bool fully_loaded = true;
				for (size_t i = 0; i < std::min(arr->size(), N); ++i)
					fully_loaded &= Serializer<T>{}.load(obj[i], TOMLNode(*arr->get(i)));
				return fully_loaded;
			}
			else
				return false;
		}

		toml::array dump(const std::array<T, N>& obj) const
		{
			toml::array arr;
			arr.reserve(N);
			for (size_t i = 0; i < N; ++i)
				arr.push_back(Serializer<T>{}.dump(obj[i]));
			return arr;
		}
	};

	template<>
	struct Serializer<Color4>
	{
		bool load(Color4& obj, TOMLNode node) const
		{
			if (auto array = node.as_array())
			{
				bool fully_loaded = true;
				for (size_t i = 0; i < std::min(array->size(), Color4::N); ++i)
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

		toml::array dump(const Color4 obj) const
		{
			toml::array arr;
			arr.reserve(Color4::N);
			for (size_t i = 0; i < Color4::N; ++i)
				arr.push_back(obj[i]);
			return arr;
		}
	};

	template<>
	struct Serializer<Rect>
	{
		bool load(Rect& obj, TOMLNode node) const
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

		toml::array dump(const Rect obj) const
		{
			toml::array arr;
			arr.reserve(Rect::N);
			for (size_t i = 0; i < Rect::N; ++i)
				arr.push_back(obj[i]);
			return arr;
		}
	};

	template<>
	struct Serializer<UVRect>
	{
		bool load(UVRect& obj, TOMLNode node) const
		{
			if (auto array = node.as_array())
			{
				bool fully_loaded = true;
				for (size_t i = 0; i < std::min(array->size(), UVRect::N); ++i)
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

		toml::array dump(const UVRect obj) const
		{
			toml::array arr;
			arr.reserve(UVRect::N);
			for (size_t i = 0; i < UVRect::N; ++i)
				arr.push_back(obj[i]);
			return arr;
		}
	};

	template<>
	struct Serializer<TopSidePadding>
	{
		bool load(TopSidePadding& obj, TOMLNode node) const
		{
			if (auto array = node.as_array())
			{
				bool fully_loaded = true;
				for (size_t i = 0; i < std::min(array->size(), TopSidePadding::N); ++i)
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

		toml::array dump(const TopSidePadding obj) const
		{
			toml::array arr;
			arr.reserve(TopSidePadding::N);
			for (size_t i = 0; i < TopSidePadding::N; ++i)
				arr.push_back(obj[i]);
			return arr;
		}
	};

	template<typename T>
	struct Serializer<std::vector<T>>
	{
		bool load(std::vector<T>& obj, TOMLNode node) const
		{
			if (auto arr = node.as_array())
			{
				obj.clear();
				bool fully_loaded = true;
				for (size_t i = 0; i < arr->size(); ++i)
				{
					T el{};
					if (Serializer<T>{}.load(el, TOMLNode(*arr->get(i))))
						obj.push_back(std::move(el));
					else
						fully_loaded = false;
				}
				return fully_loaded;
			}
			else
				return false;
		}

		toml::array dump(const std::vector<T>& obj) const
		{
			toml::array arr;
			arr.reserve(obj.size());
			for (const T& el : obj)
				arr.push_back(Serializer<T>{}.dump(el));
			return arr;
		}
	};

	template<typename T>
	struct Serializer<std::set<T>>
	{
		bool load(std::set<T>& obj, TOMLNode node) const
		{
			if (auto arr = node.as_array())
			{
				obj.clear();
				bool fully_loaded = true;
				for (size_t i = 0; i < arr->size(); ++i)
				{
					T el{};
					if (Serializer<T>{}.load(el, TOMLNode(*arr->get(i))))
						obj.insert(std::move(el));
					else
						fully_loaded = false;
				}
				return fully_loaded;
			}
			else
				return false;
		}

		toml::array dump(const std::set<T>& obj) const
		{
			toml::array arr;
			arr.reserve(obj.size());
			for (const T& el : obj)
				arr.push_back(Serializer<T>{}.dump(el));
			return arr;
		}
	};

	template<>
	struct Serializer<detail::ResourcePath>
	{
		bool load(detail::ResourcePath& obj, TOMLNode node) const
		{
			std::string path;
			if (Serializer<std::string>{}.load(path, node))
			{
				obj = std::move(path);
				return true;
			}
			else
				return false;
		}

		auto dump(const detail::ResourcePath& obj) const
		{
			return Serializer<std::string>{}.dump(obj.get_resource_shorthand());
		}
	};
}
