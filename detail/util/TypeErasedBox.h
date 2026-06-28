#pragma once

#include <memory>
#include <typeindex>

namespace oly
{
	class TypeErasedBox
	{
		void* _raw = nullptr;
		std::type_index _type;
		void(* _dtor)(void*);

	public:
		template<typename T>
		TypeErasedBox(T&& obj)
			: _type(typeid(T))
		{
			using U = std::decay_t<T>;
			_raw = new U(std::forward<T>(obj));
			_dtor = [](void* ptr) { delete static_cast<U*>(ptr); };
		}

		TypeErasedBox(const TypeErasedBox& o) = delete;

		TypeErasedBox(TypeErasedBox&& o) noexcept
			: _raw(o._raw), _type(o._type), _dtor(o._dtor)
		{
			o._raw = nullptr;
		}

		~TypeErasedBox()
		{
			_dtor(_raw);
		}

		TypeErasedBox& operator=(TypeErasedBox&& o) noexcept
		{
			if (this != &o)
			{
				_dtor(_raw);
				_raw = o._raw;
				_type = o._type;
				_dtor = o._dtor;
				o._raw = nullptr;
			}

			return *this;
		}

		template<typename T>
		std::unique_ptr<T> consume_unique()
		{
			if (typeid(T) == _type)
			{
				std::unique_ptr<T> ptr(static_cast<T*>(_raw));
				_raw = nullptr;
				return ptr;
			}
			else
				return nullptr;
		}
	};
}
