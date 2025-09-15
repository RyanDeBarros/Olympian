#pragma once

#include <stack>
#include <memory>

#include "core/base/Errors.h"

namespace oly
{
	// TODO implement ID cap for things like modulation UBO

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

	template<std::unsigned_integral T, T initial = T(0)>
	class StrictIDGenerator
	{
		friend class ID;
		T next = initial;
		std::stack<T> yielded;
		std::shared_ptr<bool> valid;

	public:
		StrictIDGenerator()
		{
			valid = std::make_shared<bool>(true);
		}

		StrictIDGenerator(const StrictIDGenerator&) = delete;
		StrictIDGenerator(StrictIDGenerator&&) = delete;

		class ID
		{
			friend class StrictIDGenerator;
			StrictIDGenerator* generator = nullptr;
			T id = T(-1);
			std::weak_ptr<bool> valid;

			ID(StrictIDGenerator& generator) : generator(&generator)
			{
				valid = generator.valid;
				if (generator.yielded.empty())
					id = generator.next++;
				else
				{
					id = generator.yielded.top();
					generator.yielded.pop();
				}
			}

		public:
			ID() = default;

			ID(const ID&) = delete;

			ID(ID&& other) noexcept : generator(other.generator), id(other.id), valid(std::move(other.valid)) { other.generator = nullptr; }

			ID& operator=(ID&& other) noexcept
			{
				if (this != &other)
				{
					if (generator)
						generator->yielded.push(id);
					generator = other.generator;
					id = other.id;
					valid = std::move(other.valid);
					other.generator = nullptr;
				}
				return *this;
			}

			~ID()
			{
				if (generator && is_valid())
					generator->yielded.push(id);
			}

			T get() const { if (is_valid()) return id; else throw Error(ErrorCode::INVALID_ID); }
			bool is_valid() const { return !valid.expired(); }
		};

		ID generate() { return ID(*this); }
	};
}
