#pragma once

#include <vector>

namespace oly
{
	template<typename T>
	class ContiguousSet
	{
		std::vector<T> set;

		typedef std::vector<T>::const_iterator Iterator;
		typedef std::vector<T>::const_reverse_iterator ReverseIterator;

	public:
		bool insert(const T& el)
		{
			if (count(el))
				return false;
			set.push_back(el);
			return true;
		}

		bool insert(T&& el)
		{
			if (count(el))
				return false;
			set.push_back(std::move(el));
			return true;
		}

		bool erase(const T& el)
		{
			auto it = find(el);
			if (it != set.end())
			{
				set.erase(it);
				return true;
			}
			else
				return false;
		}

		Iterator erase(const Iterator& where)
		{
			return set.erase(where);
		}

		Iterator remove(size_t i)
		{
			return set.erase(set.begin() + i);
		}

		bool replace(const T& existing, const T& with)
		{
			if (existing == with)
				return false;
			auto it_existing = find(existing);
			if (it_existing != set.end())
			{
				auto it_with = find(with);
				if (it_with != set.end())
					set.erase(it_existing);
				else
					*it_existing = with;
				return true;
			}
			else
				return false;
		}

		bool replace(const T& existing, T&& with)
		{
			if (existing == with)
				return false;
			auto it_existing = std::find(set.begin(), set.end(), existing);
			if (it_existing != set.end())
			{
				if (count(with))
					set.erase(it_existing);
				else
					*it_existing = std::move(with);
				return true;
			}
			else
				return false;
		}

		Iterator find(const T& el) const
		{
			return std::find(set.begin(), set.end(), el);
		}

		size_t count(const T& el) const
		{
			return find(el) != set.end() ? 1 : 0;
		}

		void clear()
		{
			set.clear();
		}

		size_t size() const
		{
			return set.size();
		}

		bool empty() const
		{
			return set.empty();
		}

		const T* data() const
		{
			return set.data();
		}

		const T& operator[](size_t i) const
		{
			return set[i];
		}

		const T& front() const
		{
			return set.front();
		}

		const T& back() const
		{
			return set.back();
		}

		T pop()
		{
			T popped = std::move(set.back());
			set.pop_back();
			return popped;
		}

		Iterator begin() const
		{
			return set.cbegin();
		}

		Iterator end() const
		{
			return set.cend();
		}

		ReverseIterator rbegin() const
		{
			return set.crbegin();
		}

		ReverseIterator rend() const
		{
			return set.crend();
		}
	};
}
