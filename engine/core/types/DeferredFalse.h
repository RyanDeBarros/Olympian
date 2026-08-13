#pragma once

#include <type_traits>

// TODO v9.3 use imp::dependent_false instead

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
