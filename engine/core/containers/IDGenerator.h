#pragma once

#include <stack>
#include <memory>

#include "core/base/Errors.h"

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

	template<std::unsigned_integral T, T initial = T(0)>
	class StrictIDGenerator
	{
		friend class ID;
		T next = initial;
		std::stack<T> yielded;
		// TODO v4 for other node systems, consider using this design of shared_ptr<Owner*> for faster move semantics, rather than using set of pointers to node. Unless of course it's necessary to refer to nodes.
		std::shared_ptr<StrictIDGenerator<T, initial>*> generator_source;

	public:
		StrictIDGenerator()
		{
			generator_source = std::make_shared<StrictIDGenerator<T, initial>*>(this);
		}

		StrictIDGenerator(const StrictIDGenerator<T, initial>&) = delete;

		StrictIDGenerator(StrictIDGenerator<T, initial>&& other) noexcept
			: next(other.next), yielded(std::move(other.yielded)), generator_source(std::move(other.generator_source))
		{
			*generator_source = this;
		}

		StrictIDGenerator& operator=(StrictIDGenerator<T, initial>&& other) noexcept
		{
			if (this != &other)
			{
				next = other.next;
				yielded = other.yielded;
				generator_source = std::move(other.generator_source);
				*generator_source = this;
			}
			return *this;
		}

		class ID
		{
			friend class StrictIDGenerator;
			std::weak_ptr<StrictIDGenerator<T, initial>*> generator;
			T id = T(-1);

			ID(StrictIDGenerator& generator) : generator(generator.generator_source)
			{
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

			ID(ID&& other) noexcept
				: generator(std::move(other.generator)), id(other.id)
			{
			}

			ID& operator=(ID&& other) noexcept
			{
				if (this != &other)
				{
					if (auto g = generator.lock())
						(*g)->yielded.push(id);
					generator = std::move(other.generator);
					id = other.id;
				}
				return *this;
			}

			~ID()
			{
				if (auto g = generator.lock())
					(*g)->yielded.push(id);
			}

			T get() const { if (is_valid()) return id; else throw Error(ErrorCode::INVALID_ID); }
			bool is_valid() const { return !generator.expired(); }
		};

		ID generate() { return ID(*this); }
	};
}
