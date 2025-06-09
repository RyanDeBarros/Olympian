#pragma once

#include <stack>

namespace oly
{
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

	public:
		class ID
		{
			friend class StrictIDGenerator;
			StrictIDGenerator* generator = nullptr;
			T id = T(-1);
			bool valid = false;

			ID(StrictIDGenerator* generator) : generator(generator)
			{
				valid = true;
				if (generator->yielded.empty())
					id = generator->next++;
				else
				{
					id = generator->yielded.top();
					generator->yielded.pop();
				}
			}

		public:
			ID() = default;
			ID(const ID&) = delete;
			ID(ID&& other) noexcept : generator(other.generator), id(other.id), valid(other.valid) { other.generator = nullptr; other.valid = false; }
			ID& operator=(ID&& other) noexcept
			{
				if (this != &other)
				{
					if (generator)
						generator->yielded.push(id);
					generator = other.generator;
					id = other.id;
					valid = other.valid;
					other.generator = nullptr;
					other.valid = false;
				}
				return *this;
			}
			~ID()
			{
				if (generator && valid)
					generator->yielded.push(id);
			}

			T get() const { return id; }
			bool is_valid() const { return valid; }
		};

		ID generate() { return ID(this); }
	};
}
