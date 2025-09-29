#pragma once

#include <stack>

#include "core/base/Errors.h"
#include "core/types/Issuer.h"

namespace oly
{
	template<std::unsigned_integral T>
	class SoftIDGenerator
	{
		T next;
		T max;
		std::stack<T> yielded;

	public:
		SoftIDGenerator(T initial = T(0), T max = nmax<T>())
			: next(initial), max(max)
		{
		}

		T gen()
		{
			if (yielded.empty())
			{
				if (next > max)
					throw Error(ErrorCode::INDEX_OUT_OF_RANGE);
				return next++;
			}
			T id = yielded.top();
			yielded.pop();
			return id;
		}
		void yield(T id)
		{
			yielded.push(id);
		}
	};

	namespace internal
	{
		template<std::unsigned_integral T>
		class StrictIDGenerator : public Issuer<StrictIDGenerator<T>>
		{
			using Super = Issuer<StrictIDGenerator<T>>;
			friend class ID;
			T next;
			T max;
			std::stack<T> yielded;

		public:
			StrictIDGenerator(T initial, T max)
				: next(initial), max(max)
			{
			}

			StrictIDGenerator(const StrictIDGenerator<T>&) = delete;

			StrictIDGenerator(StrictIDGenerator<T>&& other) noexcept = default;

			StrictIDGenerator<T>& operator=(StrictIDGenerator<T>&& other) noexcept = default;

			class ID : public Issuer<StrictIDGenerator<T>>::Handle
			{
				using Super = Issuer<StrictIDGenerator<T>>::Handle;
				friend class StrictIDGenerator<T>;
				T id = T(-1);

				void init()
				{
					auto accessor = Super::lock();
					if (StrictIDGenerator<T>* generator = accessor.get())
					{
						if (generator->yielded.empty())
						{
							if (generator->next > generator->max)
								throw Error(ErrorCode::INDEX_OUT_OF_RANGE);
							id = generator->next++;
						}
						else
						{
							id = generator->yielded.top();
							generator->yielded.pop();
						}
					}
				}

				void del()
				{
					auto accessor = Super::lock();
					if (StrictIDGenerator<T>* generator = accessor.get())
						generator->yielded.push(id);
					Super::reset();
					id = T(-1);
				}

				ID(Super&& super)
					: Super(std::move(super))
				{
					init();
				}

			public:
				ID() = default;

				ID(const ID& other)
					: Super(other)
				{
					init();
				}

				ID(ID&& other) noexcept
					: Super(std::move(other)), id(other.id)
				{
					other.id = T(-1);
				}

				ID& operator=(const ID& other)
				{
					if (this != &other)
					{
						del();
						Super::operator=(other);
						init();
					}
				}

				ID& operator=(ID&& other) noexcept
				{
					if (this != &other)
					{
						del();
						Super::operator=(std::move(other));
						id = other.id;
						other.id = T(-1);
					}
					return *this;
				}

				~ID()
				{
					del();
				}

				T get() const
				{
					if (Super::is_valid())
						return id;
					else
						throw Error(ErrorCode::INVALID_ID);
				}
				
				void yield()
				{
					del();
				}
			};

			ID generate() { return Super::issue(); }
		};
	}

	template<std::unsigned_integral T>
	using StrictIDGenerator = PublicIssuer<internal::StrictIDGenerator<T>>;
}
