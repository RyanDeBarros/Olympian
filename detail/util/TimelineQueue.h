#pragma once

#include <optional>
#include <vector>

// TODO v9.3 move to imp

namespace oly
{
	template<typename T>
	class TimelineQueue
	{
		size_t _limit;
		std::vector<T> _past;
		std::optional<T> _present;
		std::vector<T> _future;

	public:
		TimelineQueue(size_t limit)
			: _limit(limit)
		{
		}

		void push(T obj)
		{
			if (_present)
				_past.push_back(std::move(*_present));
			_present = std::move(obj);
			_future.clear();
			prune();
		}

		void move_backward()
		{
			if (_present)
			{
				_future.push_back(std::move(*_present));
				_present.reset();
			}

			if (!_past.empty())
			{
				_present = std::move(_past.back());
				_past.pop_back();
			}
		}

		void move_forward()
		{
			if (_present)
			{
				_past.push_back(std::move(*_present));
				_present.reset();
			}

			if (!_future.empty())
			{
				_present = std::move(_future.back());
				_future.pop_back();
			}
		}

		const T* get_present() const
		{
			return _present ? &*_present : nullptr;
		}

		T* get_present()
		{
			return _present ? &*_present : nullptr;
		}

		void set_limit(size_t limit)
		{
			_limit = limit;
			prune();
		}

		void prune()
		{
			if (_future.size() + 1 >= _limit)
				_past.clear();
			else
			{
				const size_t past_limit = _limit - _future.size() - 1;
				if (_past.size() > past_limit)
					_past.erase(_past.begin(), _past.end() - past_limit);
			}
		}

		void clear()
		{
			_past.clear();
			_present.reset();
			_future.clear();
		}

		bool empty_backwards() const
		{
			return _past.empty();
		}

		bool empty_forwards() const
		{
			return _future.empty();
		}
	};
}
