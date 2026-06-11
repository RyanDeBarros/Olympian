#pragma once

#include <utility>

namespace oly::editor
{
	template<typename T>
	class Modifiable
	{
		bool _dirty = false;
		T _obj;

	public:
		Modifiable(T ini = T()) : _obj(ini) {}
		
		Modifiable(const Modifiable& o)
		{
			Set(o._obj);
		}

		Modifiable(Modifiable&& o) noexcept
			: _dirty(o._dirty), _obj(std::move(o._obj))
		{
		}

		Modifiable& operator=(const Modifiable& o)
		{
			if (this != &o)
				Set(o._obj);

			return *this;
		}

		Modifiable& operator=(Modifiable&& o)
		{
			if (this != &o)
			{
				_dirty = o._dirty;
				_obj = std::move(o._obj);
			}

			return *this;
		}

		Modifiable& operator=(T val)
		{
			Set(val);
			return *this;
		}

		operator T() const
		{
			return Get();
		}

		T Get() const
		{
			return _obj;
		}

		void Set(T val)
		{
			if (_obj != val)
			{
				_obj = val;
				_dirty = true;
			}
		}

		bool WasModified()
		{
			return _dirty;
		}

		bool ConsumeModified()
		{
			if (_dirty)
			{
				_dirty = false;
				return true;
			}
			else
				return false;
		}
	};
}
