#pragma once

#include "core/base/Errors.h"

#include <tuple>

namespace oly
{
	template<typename... Containers>
	class Zipper
	{
		static_assert(sizeof...(Containers) > 0);

		std::tuple<Containers&...> containers;

	public:
		Zipper(Containers&... containers) : containers(containers...) {}

		class Iterator
		{
			template<typename T>
			using ContainerIterator = decltype(std::begin(std::declval<T&>()));

			std::tuple<ContainerIterator<Containers>...> iterators;

		public:
			Iterator(ContainerIterator<Containers>... iterators) : iterators(iterators...) {}

		private:
			template<size_t... Indexes>
			bool neq_impl(const Iterator& other, std::index_sequence<Indexes...>) const
			{
				return (... && (std::get<Indexes>(iterators) != std::get<Indexes>(other.iterators)));
			}

		public:
			bool operator!=(const Iterator& other) const
			{
				return neq_impl(other, std::index_sequence_for<Containers...>{});
			}

			Iterator& operator++()
			{
				std::apply([](auto&... iterators) { ((++iterators), ...); }, iterators);
				return *this;
			}

			auto operator*() const
			{
				return std::apply([](auto&... iterators) { return std::tie(*iterators...); }, iterators);
			}
		};

		Iterator begin()
		{
			return std::apply([](auto&... containers) { return Iterator(std::begin(containers)...); }, containers);
		}

		Iterator end()
		{
			return std::apply([](auto&... containers) { return Iterator(std::end(containers)...); }, containers);
		}
	};

	template<bool Strict = true, typename... Containers>
	inline auto zip(Containers&&... containers)
	{
		if constexpr (Strict)
		{
			const auto first_size = std::size(std::get<0>(std::tie(containers...)));
			if (((std::size(containers) != first_size) || ...))
				throw Error(ErrorCode::CONDITION_FAILED);
		}

		return Zipper(std::forward<Containers>(containers)...);
	}
}
