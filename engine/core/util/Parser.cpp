#include "Parser.h"

#include "core/base/Errors.h"
#include "core/base/Color.h"
#include "core/util/Logger.h"
#include "core/util/UTF.h"
#include "core/cmath/ColoredGeometry.h"

#include "definitions/Keys.h"
#include "util/Parser.h"

namespace oly::assets
{
	detail::Key NO_KEY = detail::Key::_;
}

namespace oly::assets::internal
{
	template<>
	bool try_parse(TOMLNode node, bool& v)
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
	bool try_parse(TOMLNode node, int& v)
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
	bool try_parse(TOMLNode node, unsigned int& v)
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
	bool try_parse(TOMLNode node, unsigned char& v)
	{
		if (auto i = node.value<int64_t>())
		{
			v = (unsigned char)*i;
			return true;
		}
		else
			return false;
	}

	template<>
	bool try_parse(TOMLNode node, float& v)
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
	bool try_parse(TOMLNode node, double& v)
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
	bool try_parse(TOMLNode node, size_t& v)
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
	bool try_parse(TOMLNode node, glm::vec2& v)
	{
		return try_parse_vec<2, float, true>(node, v);
	}

	template<>
	bool try_parse(TOMLNode node, glm::vec3& v)
	{
		return try_parse_vec<3, float, true>(node, v);
	}

	template<>
	bool try_parse(TOMLNode node, glm::vec4& v)
	{
		return try_parse_vec<4, float, true>(node, v);
	}

	template<>
	bool try_parse(TOMLNode node, Color& c)
	{
		glm::vec4 v = c;
		if (try_parse(node, v))
		{
			c = v;
			return true;
		}
		else
			return false;
	}

	template<>
	bool try_parse(TOMLNode node, PartialView<glm::vec2> v)
	{
		return try_parse_vec<2, float, false>(node, v.val);
	}

	template<>
	bool try_parse(TOMLNode node, PartialView<glm::vec3> v)
	{
		return try_parse_vec<3, float, false>(node, v.val);
	}

	template<>
	bool try_parse(TOMLNode node, PartialView<glm::vec4> v)
	{
		return try_parse_vec<4, float, false>(node, v.val);
	}

	template<>
	bool try_parse(TOMLNode node, glm::ivec2& v)
	{
		return try_parse_vec<2, int, true>(node, v);
	}

	template<>
	bool try_parse(TOMLNode node, glm::ivec3& v)
	{
		return try_parse_vec<3, int, true>(node, v);
	}

	template<>
	bool try_parse(TOMLNode node, glm::ivec4& v)
	{
		return try_parse_vec<4, int, true>(node, v);
	}

	template<>
	bool try_parse(TOMLNode node, PartialView<glm::ivec2> v)
	{
		return try_parse_vec<2, int, false>(node, v.val);
	}

	template<>
	bool try_parse(TOMLNode node, PartialView<glm::ivec3> v)
	{
		return try_parse_vec<3, int, false>(node, v.val);
	}

	template<>
	bool try_parse(TOMLNode node, PartialView<glm::ivec4> v)
	{
		return try_parse_vec<4, int, false>(node, v.val);
	}

	template<>
	bool try_parse(TOMLNode node, std::array<bool, 3>& v)
	{
		return try_parse_array<3, bool, true>(node, v);
	}

	template<>
	bool try_parse(TOMLNode node, PartialView<std::array<bool, 3>> v)
	{
		return try_parse_array<3, bool, false>(node, v.val);
	}

	template<>
	bool try_parse(TOMLNode node, std::string& v)
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
	bool try_parse(TOMLNode node, TOMLArray& v)
	{
		if (auto o = node.as_array())
		{
			v = o;
			return true;
		}
		else
			return false;
	}

	template<>
	bool try_parse(TOMLNode node, utf::Codepoint& v)
	{
		std::string str;
		if (!try_parse(node, str))
			return false;

		if (auto val = stocdpt(str))
		{
			v = utf::Codepoint(*val);
			return true;
		}
		else
			return false;
	}

	template<>
	bool try_parse(TOMLNode node, cmath::BorderPivot& v)
	{
		// TODO v11 in editor, either select from combo box or set float directly
		return try_parse(node, v.v);
	}

	void log_context_at_level(LogLevel level, const DeferredStringParam& msg, std::source_location location)
	{
		OLY_LOG_AT_LEVEL(level, true, "CONTEXT") << LOG.source_info.full_source(location) << msg.str() << LOG.nl;
	}
}
