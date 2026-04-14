#include "Parser.h"

#include "core/base/Errors.h"
#include "core/util/Logger.h"

namespace oly::io::internal
{
	template<>
	bool try_parse<bool>(TOMLNode node, bool& v)
	{
		if (auto i = node.value<bool>())
		{
			v = *i;
			return true;
		}
		else if (auto i = node.value<int64_t>())
		{
			v = (bool)*i;
			return true;
		}
		else
			return false;
	}

	template<>
	bool try_parse<int>(TOMLNode node, int& v)
	{
		if (auto i = node.value<int64_t>())
		{
			v = (int)*i;
			return true;
		}
		else
			return false;
	}

	template<>
	bool try_parse<unsigned int>(TOMLNode node, unsigned int& v)
	{
		if (auto i = node.value<int64_t>())
		{
			v = (unsigned int)*i;
			return true;
		}
		else
			return false;
	}

	template<>
	bool try_parse<float>(TOMLNode node, float& v)
	{
		if (auto i = node.value<double>())
		{
			v = (float)*i;
			return true;
		}
		else if (auto i = node.value<int64_t>())
		{
			v = (float)*i;
			return true;
		}
		else
			return false;
	}

	template<>
	bool try_parse<double>(TOMLNode node, double& v)
	{
		if (auto i = node.value<double>())
		{
			v = *i;
			return true;
		}
		else if (auto i = node.value<int64_t>())
		{
			v = (double)*i;
			return true;
		}
		else
			return false;
	}

	template<>
	bool try_parse<size_t>(TOMLNode node, size_t& v)
	{
		if (auto i = node.value<int64_t>())
		{
			v = (size_t)*i;
			return true;
		}
		else
			return false;
	}

	template<size_t N, typename T, bool StrictSize>
	bool try_parse_vec(TOMLNode node, glm::vec<N, T>& v)
	{
		auto arr = node.as_array();
		if (arr && (StrictSize ? arr->size() == N : arr->size() <= N))
		{
			glm::vec<N, T> u = v;
			for (int i = 0; i < arr->size(); ++i)
				if (!try_parse((TOMLNode)*arr->get(i), u[i]))
					return false;
			v = u;
			return true;
		}
		else
			return false;
	}

	template<size_t N, typename T, bool StrictSize>
	bool try_parse_array(TOMLNode node, std::array<T, N>& v)
	{
		auto arr = node.as_array();
		if (arr && (StrictSize ? arr->size() == N : arr->size() <= N))
		{
			std::array<T, N> u = v;
			for (int i = 0; i < arr->size(); ++i)
				if (!try_parse((TOMLNode)*arr->get(i), u[i]))
					return false;
			v = u;
			return true;
		}
		else
			return false;
	}

	template<>
	bool try_parse<glm::vec2>(TOMLNode node, glm::vec2& v)
	{
		return try_parse_vec<2, float, true>(node, v);
	}

	template<>
	bool try_parse<glm::vec3>(TOMLNode node, glm::vec3& v)
	{
		return try_parse_vec<3, float, true>(node, v);
	}

	template<>
	bool try_parse<glm::vec4>(TOMLNode node, glm::vec4& v)
	{
		return try_parse_vec<4, float, true>(node, v);
	}

	template<>
	bool try_parse<glm::vec2>(TOMLNode node, PartialView<glm::vec2> v)
	{
		return try_parse_vec<2, float, false>(node, v.val);
	}

	template<>
	bool try_parse<glm::vec3>(TOMLNode node, PartialView<glm::vec3> v)
	{
		return try_parse_vec<3, float, false>(node, v.val);
	}

	template<>
	bool try_parse<glm::vec4>(TOMLNode node, PartialView<glm::vec4> v)
	{
		return try_parse_vec<4, float, false>(node, v.val);
	}

	template<>
	bool try_parse<glm::ivec2>(TOMLNode node, glm::ivec2& v)
	{
		return try_parse_vec<2, int, true>(node, v);
	}

	template<>
	bool try_parse<glm::ivec3>(TOMLNode node, glm::ivec3& v)
	{
		return try_parse_vec<3, int, true>(node, v);
	}

	template<>
	bool try_parse<glm::ivec4>(TOMLNode node, glm::ivec4& v)
	{
		return try_parse_vec<4, int, true>(node, v);
	}

	template<>
	bool try_parse<glm::ivec2>(TOMLNode node, PartialView<glm::ivec2> v)
	{
		return try_parse_vec<2, int, false>(node, v.val);
	}

	template<>
	bool try_parse<glm::ivec3>(TOMLNode node, PartialView<glm::ivec3> v)
	{
		return try_parse_vec<3, int, false>(node, v.val);
	}

	template<>
	bool try_parse<glm::ivec4>(TOMLNode node, PartialView<glm::ivec4> v)
	{
		return try_parse_vec<4, int, false>(node, v.val);
	}

	template<>
	bool try_parse<std::array<bool, 3>>(TOMLNode node, std::array<bool, 3>& v)
	{
		return try_parse_array<3, bool, true>(node, v);
	}

	template<>
	bool try_parse<std::array<bool, 3>>(TOMLNode node, PartialView<std::array<bool, 3>> v)
	{
		return try_parse_array<3, bool, false>(node, v.val);
	}

	template<>
	bool try_parse<std::string>(TOMLNode node, std::string& v)
	{
		if (auto s = node.value<std::string>())
		{
			v = *s;
			return true;
		}
		else
			return false;
	}

	template<>
	bool try_parse<TOMLArray>(TOMLNode node, TOMLArray& v)
	{
		if (auto o = node.as_array())
		{
			v = o;
			return true;
		}
		else
			return false;
	}

	void log_context_at_level(LogLevel level, const DeferredStringParam& msg, std::source_location location)
	{
		OLY_LOG_AT_LEVEL(level, true, "CONTEXT") << LOG.source_info.full_source(location) << msg.str() << LOG.nl;
	}
}
