#pragma once

#include <stack>

#include "core/base/Errors.h"
#include "core/types/Issuer.h"

namespace oly
{
	// TODO v4 implement ID cap for things like modulation UBO

	template<std::unsigned_integral T>
	class SoftIDGenerator
	{
		T next = 0;
		std::stack<T> yielded;

	public:
		T gen()
		{
			if (yielded.empty())
				return next++;
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
		// StrictIDGenerator must be allocated on heap.
		template<std::unsigned_integral T>
		class StrictIDGenerator : public Issuer<StrictIDGenerator<T>>
		{
			using Super = Issuer<StrictIDGenerator<T>>;
			friend class ID;
			T next;
			std::stack<T> yielded;

		public:
			StrictIDGenerator(T initial)
				: next(initial)
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

				ID(Super&& super)
					: Super(std::move(super))
				{
					auto accessor = Super::lock();
					if (StrictIDGenerator<T>* generator = accessor.get())
					{
						if (generator->yielded.empty())
							id = generator->next++;
						else
						{
							id = generator->yielded.top();
							generator->yielded.pop();
						}
					}
				}

			public:
				ID() = default;

				ID(const ID&) = delete;

				ID(ID&&) noexcept = default;

				ID& operator=(const ID&) = delete;

				ID& operator=(ID&& other) noexcept
				{
					if (this != &other)
					{
						{
							auto accessor = Super::lock();
							if (StrictIDGenerator<T>* generator = accessor.get())
								generator->yielded.push(id);
						}
						Super::operator=(std::move(other));
						id = other.id;
					}
					return *this;
				}

				~ID()
				{
					auto accessor = Super::lock();
					if (StrictIDGenerator<T>* generator = accessor.get())
						generator->yielded.push(id);
				}

				T get() const { if (Super::is_valid()) return id; else throw Error(ErrorCode::INVALID_ID); }
			};

			ID generate() { return Super::issue(); }
		};
	}

	template<std::unsigned_integral T>
	class StrictIDGenerator
	{
		using Generator = internal::StrictIDGenerator<T>;
		std::shared_ptr<Generator> generator;

	public:
		using ID = Generator::ID;

		StrictIDGenerator(T initial = T(0))
		{
			generator = std::make_shared<Generator>(initial);
		}

		StrictIDGenerator(const StrictIDGenerator&) = delete;
		StrictIDGenerator(StrictIDGenerator&&) = default;

		ID generate() { return generator->generate(); }
	};
}
