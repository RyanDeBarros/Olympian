#pragma once

#include "core/base/Errors.h"

#include <tuple>

namespace oly
{
	template<typename T>
	using ContainerIterator = decltype(std::begin(std::declval<T&>()));

	template<typename T>
	using ConstContainerIterator = decltype(std::begin(std::declval<const T&>()));

	template<typename... Containers>
	class Zipper
	{
		static_assert(sizeof...(Containers) > 0);

		std::tuple<Containers&...> containers;

	public:
		Zipper(Containers&... containers) : containers(containers...) {}

		class Iterator
		{
			std::tuple<ContainerIterator<Containers>...> iterators;

		public:
			Iterator(ContainerIterator<Containers>... iterators) : iterators(std::tie(iterators...)) {}

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

	template<typename... Containers>
	class ConstZipper
	{
		static_assert(sizeof...(Containers) > 0);

		std::tuple<const Containers&...> containers;

	public:
		ConstZipper(const Containers&... containers) : containers(containers...) {}

		class ConstIterator
		{
			std::tuple<ConstContainerIterator<Containers>...> iterators;

		public:
			ConstIterator(ConstContainerIterator<Containers>... iterators) : iterators(std::tie(iterators...)) {}

		private:
			template<size_t... Indexes>
			bool neq_impl(const ConstIterator& other, std::index_sequence<Indexes...>) const
			{
				return (... && (std::get<Indexes>(iterators) != std::get<Indexes>(other.iterators)));
			}

		public:
			bool operator!=(const ConstIterator& other) const
			{
				return neq_impl(other, std::index_sequence_for<Containers...>{});
			}

			ConstIterator& operator++()
			{
				std::apply([](auto&... iterators) { ((++iterators), ...); }, iterators);
				return *this;
			}

			auto operator*() const
			{
				return std::apply([](const auto&... iterators) { return std::tie(*iterators...); }, iterators);
			}
		};

		ConstIterator begin() const
		{
			return std::apply([](const auto&... containers) { return ConstIterator(std::cbegin(containers)...); }, containers);
		}

		ConstIterator end() const
		{
			return std::apply([](const auto&... containers) { return ConstIterator(std::cend(containers)...); }, containers);
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

		if constexpr ((std::is_const_v<std::remove_reference_t<Containers>> && ...))
			return ConstZipper(std::forward<Containers>(containers)...);
		else
			return Zipper(std::forward<Containers>(containers)...);
	}
}
