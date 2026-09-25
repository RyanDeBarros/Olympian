#pragma once

#include "physics/collision/methods/CollisionInfo.h"
#include "physics/collision/objects/Combinations.h"

#include "core/math/Shapes.h"

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
        imp::box _obj;

	public:
		template<CObj_check CObj>
		ColliderObject(CObj&& obj) : _obj(imp::forward_to_box(std::forward<CObj>(obj))) {}

		ColliderObject()
            : _obj(imp::make_box<TPrimitive>())
        {}
		
		const void* raw_obj() const
        {
            return _obj.unsafe_raw();
        }

        void* raw_obj()
        {
            return _obj.unsafe_raw();
        }
		
        size_t type_index() const
        {
            return CObj_index(_obj.type());
        }

		template<CObj_check CObj>
		const CObj& get() const
		{
            if (auto obj = _obj.as<CObj>())
                return *obj;
			else
				throw Error(ErrorCode::INVALID_TYPE);
		}

		template<CObj_check CObj>
		CObj& set()
		{
            if (auto obj = _obj.as<CObj>())
                return *obj;
			else
				throw Error(ErrorCode::INVALID_TYPE);
		}
	};
}
