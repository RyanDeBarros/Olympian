#pragma once

#include "core/base/Errors.h"

#include <tuple>

namespace oly
{
	template<typename T, typename U>
	class Zipper
	{
		using TIterator = decltype(std::begin(std::declval<T&>()));
		using UIterator = decltype(std::begin(std::declval<U&>()));

		T& t;
		U& u;

	public:
		Zipper(T& t, U& u, bool assert_same_size = true)
			: t(t), u(u)
		{
			if (assert_same_size && std::size(t) != std::size(u))
				throw Error(ErrorCode::CONDITION_FAILED);
		}

		class Iterator
		{
			TIterator titer;
			UIterator uiter;

		public:
			Iterator(TIterator titer, UIterator uiter) : titer(titer), uiter(uiter) {}

			bool operator!=(const Iterator& other) const
			{
				return titer != other.titer && uiter != other.uiter;
			}

			Iterator& operator++()
			{
				++titer;
				++uiter;
				return *this;
			}

			Iterator& operator++(int)
			{
				Iterator it = *this;
				++*this;
				return it;
			}

			auto operator*() const
			{
				return std::tie(*titer, *uiter);
			}
		};

		Iterator begin() const
		{
			return { std::begin(t), std::begin(u) };
		}

		Iterator end() const
		{
			return { std::end(t), std::end(u) };
		}
	};

	template<typename T, typename U>
	class ConstZipper
	{
		using TIterator = decltype(std::cbegin(std::declval<const T&>()));
		using UIterator = decltype(std::cbegin(std::declval<const U&>()));

		const T& t;
		const U& u;

	public:
		ConstZipper(const T& t, const U& u, bool assert_same_size = true)
			: t(t), u(u)
		{
			if (assert_same_size && std::size(t) != std::size(u))
				throw Error(ErrorCode::CONDITION_FAILED);
		}

		class ConstIterator
		{
			TIterator titer;
			UIterator uiter;

		public:
			ConstIterator(TIterator titer, UIterator uiter) : titer(titer), uiter(uiter) {}

			bool operator!=(const ConstIterator& other) const
			{
				return titer != other.titer && uiter != other.uiter;
			}

			ConstIterator& operator++()
			{
				++titer;
				++uiter;
				return *this;
			}

			ConstIterator& operator++(int)
			{
				ConstIterator it = *this;
				++*this;
				return it;
			}

			auto operator*() const
			{
				return std::tie(*titer, *uiter);
			}
		};

		ConstIterator begin() const
		{
			return { std::cbegin(t), std::cbegin(u) };
		}

		ConstIterator end() const
		{
			return { std::cend(t), std::cend(u) };
		}
	};
}
