#pragma once

#include "physics/collision/methods/CollisionInfo.h"
#include "physics/collision/objects/Combinations.h"

#include "core/math/Shapes.h"
#include "core/containers/BlackBox.h"

#include <imp/type_delimiter.hpp>

namespace oly::col2d::internal
{
#define _OLY_COBJ_GENERATOR(M) \
        M((TPrimitive)) \
        M((TCompound)) \
        M((TBVH<AABB>)) \
        M((TBVH<OBB>)) \
        M((TBVH<KDOP2>)) \
        M((TBVH<KDOP3>)) \
        M((TBVH<KDOP4>)) \
        M((TBVH<KDOP5>)) \
        M((TBVH<KDOP6>)) \
        M((TBVH<KDOP7>)) \
        M((TBVH<KDOP8>))

    IMP_TYPE_DELIMITER(_OLY_COBJ_GENERATOR, CObj);

	class ColliderObject
	{
		BlackBox _obj;
		imp::type_erasure _type;

	public:
		template<CObj_check CObj>
		ColliderObject(CObj&& obj) : _obj(std::forward<CObj>(obj)), _type(imp::erase_type<CObj>()) {}

		ColliderObject()
            : _obj(TPrimitive()), _type(imp::erase_type<TPrimitive>())
        {}
		
        ColliderObject(const ColliderObject& o)
            : _obj(o._obj), _type(o._type)
        {}
		
        ColliderObject(ColliderObject&& o) noexcept
            : _obj(std::move(o._obj)), _type(o._type)
        {}
		
        ColliderObject& operator=(const ColliderObject& o)
        {
            if (this != &o)
            {
                _obj = o._obj;
                _type = o._type;
            }
            return *this;
        }
		
        ColliderObject& operator=(ColliderObject&& o) noexcept
        {
            if (this != &o)
            {
                _obj = std::move(o._obj);
                _type = o._type;
            }
            return *this;
        }

		const void* raw_obj() const
        {
            return _obj.raw();
        }

        void* raw_obj()
        {
            return _obj.raw();
        }
		
        size_t type_index() const
        {
            return CObj_index(_type);
        }

		template<CObj_check CObj>
		const CObj& get() const
		{
			if (imp::erase_type<CObj>() == _type)
				return *_obj.cast<CObj>();
			else
				throw Error(ErrorCode::INVALID_TYPE);
		}

		template<CObj_check CObj>
		CObj& set()
		{
			if (imp::erase_type<CObj>() == _type)
				return *_obj.cast<CObj>();
			else
				throw Error(ErrorCode::INVALID_TYPE);
		}
	};
}
