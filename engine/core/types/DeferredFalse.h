#pragma once

#include <type_traits>

namespace oly
{
	namespace internal
	{
		template<auto>
		struct DeferredFalse : public std::false_type {};
	}

	template<auto T>
	constexpr bool deferred_false = internal::DeferredFalse<T>::value;
}
