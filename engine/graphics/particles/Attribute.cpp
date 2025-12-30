#include "Attribute.h"

#include "graphics/particles/distributions/AttributeOperations.h"
#include "core/algorithms/STLUtils.h"

namespace oly::particles
{
//	static Polymorphic<IAttributeOperation> load_selector(TOMLNode node)
//	{
//		// TODO v6 better selector (load) system? Move away from templates?
//		auto s = node["selector"].value<std::string>();
//		if (!s)
//			return nullptr;
//		std::string selector = algo::to_lower(std::move(*s));
//
//#define _OLY_MAKE_SELECTOR(T, U, ...) make_polymorphic<operations::Selector<T, U>>(IAttributeOperation<U>::load(node["inner_op"]), [](T v) -> U { return OLY_FLATTEN(__VA_ARGS__); })
//
//		if constexpr (std::is_same_v<T, glm::vec2>)
//		{
//			if (selector == ".x")
//				return _OLY_MAKE_SELECTOR(glm::vec2, float, v.x);
//			else if (selector == ".y")
//				return _OLY_MAKE_SELECTOR(glm::vec2, float, v.y);
//		}
//		else if constexpr (std::is_same_v<T, glm::vec3>)
//		{
//			if (selector == ".x")
//				return _OLY_MAKE_SELECTOR(glm::vec3, float, v.x);
//			else if (selector == ".y")
//				return _OLY_MAKE_SELECTOR(glm::vec3, float, v.y);
//			else if (selector == ".z")
//				return _OLY_MAKE_SELECTOR(glm::vec3, float, v.z);
//			else if (selector == ".xy")
//				return _OLY_MAKE_SELECTOR(glm::vec3, glm::vec2, { v.x, v.y });
//			else if (selector == ".xz")
//				return _OLY_MAKE_SELECTOR(glm::vec3, glm::vec2, { v.x, v.z });
//			else if (selector == ".yz")
//				return _OLY_MAKE_SELECTOR(glm::vec3, glm::vec2, { v.y, v.z });
//		}
//		else if constexpr (std::is_same_v<T, glm::vec4>)
//		{
//			if (selector == ".x")
//				return _OLY_MAKE_SELECTOR(glm::vec4, float, v.x);
//			else if (selector == ".y")
//				return _OLY_MAKE_SELECTOR(glm::vec4, float, v.y);
//			else if (selector == ".z")
//				return _OLY_MAKE_SELECTOR(glm::vec4, float, v.z);
//			else if (selector == ".w")
//				return _OLY_MAKE_SELECTOR(glm::vec4, float, v.w);
//			else if (selector == ".xy")
//				return _OLY_MAKE_SELECTOR(glm::vec4, glm::vec2, { v.x, v.y });
//			else if (selector == ".xz")
//				return _OLY_MAKE_SELECTOR(glm::vec4, glm::vec2, { v.x, v.z });
//			else if (selector == ".xw")
//				return _OLY_MAKE_SELECTOR(glm::vec4, glm::vec2, { v.x, v.w });
//			else if (selector == ".yz")
//				return _OLY_MAKE_SELECTOR(glm::vec4, glm::vec2, { v.y, v.z });
//			else if (selector == ".yw")
//				return _OLY_MAKE_SELECTOR(glm::vec4, glm::vec2, { v.y, v.w });
//			else if (selector == ".zw")
//				return _OLY_MAKE_SELECTOR(glm::vec4, glm::vec2, { v.z, v.w });
//			else if (selector == ".xyz")
//				return _OLY_MAKE_SELECTOR(glm::vec4, glm::vec3, { v.x, v.y, v.z });
//			else if (selector == ".xyw")
//				return _OLY_MAKE_SELECTOR(glm::vec4, glm::vec3, { v.x, v.y, v.w });
//			else if (selector == ".xzw")
//				return _OLY_MAKE_SELECTOR(glm::vec4, glm::vec3, { v.x, v.z, v.w });
//			else if (selector == ".yzw")
//				return _OLY_MAKE_SELECTOR(glm::vec4, glm::vec3, { v.y, v.z, v.w });
//		}
//
//#undef _OLY_MAKE_SELECTOR
//
//		return nullptr;
//	}
//
//	Polymorphic<IAttributeOperation> IAttributeOperation::load(TOMLNode node)
//	{
//		if (!node)
//			return nullptr;
//
//		std::string op = algo::to_lower(node["op"].value_or<std::string>(""));
//
//		if (op == "sequence")
//			return operations::Sequence::load(node);
//		else if (op == "selector")
//			return load_selector(node);
//		else if (op == "genericfunction")
//			return operations::GenericFunction::load(node);
//		else if constexpr (std::is_same_v<T, float>)
//		{
//			if (op == "sinewave1d")
//				return operations::SineWave1D::load(node);
//		}
//		else if constexpr (std::is_same_v<T, glm::vec2>)
//		{
//			if (op == "polarization2d")
//				return operations::Polarization2D::load(node);
//		}
//
//		return nullptr;
//	}
}
