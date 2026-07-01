#pragma once

#include <functional>
#include <unordered_map>
#include <stack>

namespace oly
{
	template<typename... Args>
	class FunctionalEvent
	{
	public:
		class Handle
		{
			FunctionalEvent<Args...>* _event = nullptr;
			size_t _h = 0;

			friend class FunctionalEvent<Args...>;

			Handle(FunctionalEvent<Args...>* event, size_t h) : _event(event), _h(h) {}

		public:
			Handle() = default;

			Handle(const Handle&) = delete;
			Handle(Handle&& o) noexcept
				: _event(o._event), _h(o._h)
			{
				o._event = nullptr;

				if (_event)
				{
					auto it = _event->_listeners.find(_h);
					if (it != _event->_listeners.end())
						it->second.handle = this;
				}
			}

			~Handle()
			{
				if (_event)
					_event->unsubscribe(*this);
			}

			Handle& operator=(Handle&& o) noexcept
			{
				if (this != &o)
				{
					if (_event)
						_event->unsubscribe(*this);

					_event = o._event;
					_h = o._h;

					o._event = nullptr;

					if (_event)
					{
						auto it = _event->_listeners.find(_h);
						if (it != _event->_listeners.end())
							it->second.handle = this;
					}
				}

				return *this;
			}
		};

		using Callback = std::function<void(Args...)>;

	private:
		struct Listener
		{
			Handle* handle;
			Callback callback;

			Listener(Handle* handle, Callback callback)
				: handle(handle), callback(std::move(callback))
			{
			}
		};

		std::unordered_map<size_t, Listener> _listeners;
		std::stack<size_t> _free_handles;

	public:
		FunctionalEvent() = default;
		FunctionalEvent(const FunctionalEvent&) = delete;
		FunctionalEvent(FunctionalEvent&&) = delete;

		~FunctionalEvent()
		{
			clear();
		}

		Handle subscribe(Callback callback)
		{
			size_t h;
			if (_free_handles.empty())
				h = _listeners.size();
			else
			{
				h = _free_handles.top();
				_free_handles.pop();
			}

			Handle handle(this, h);
			_listeners.emplace(h, Listener(&handle, std::move(callback)));
			return handle;
		}
		
		bool unsubscribe(Handle& handle)
		{
			if (this == handle._event)
			{
				auto it = _listeners.find(handle._h);
				if (it == _listeners.end())
					return false;

				_listeners.erase(it);
				_free_handles.push(handle._h);

				handle._event = nullptr;
				return true;
			}
			else
				return false;
		}

		void invoke(Args... args)
		{
			for (auto& [h, listener] : _listeners)
				listener.callback(args...);
		}

		void clear()
		{
			for (auto& [h, listener] : _listeners)
				listener.handle->_event = nullptr;

			_listeners.clear();

			while (!_free_handles.empty())
				_free_handles.pop();
		}
	};
}
