#pragma once

#include "physics/collision/methods/CollisionInfo.h"
#include "physics/collision/objects/Combinations.h"

#include "core/math/Shapes.h"
#include "core/containers/BlackBox.h"

namespace oly::col2d::internal
{
	enum CObjID : unsigned int;

	template<typename T>
	struct CObjIDTrait;

	class ColliderObject
	{
		BlackBox<true> _obj;
		CObjID _id;

	public:
		template<typename CObj>
		explicit ColliderObject(CObj&& obj)
			: _obj(std::forward<CObj>(obj)), _id(CObjIDTrait<std::decay_t<CObj>>::ID)
		{
		}

		const void* raw_obj() const { return _obj.raw(); }
		CObjID id() const { return _id; }

		template<typename CObj>
		const CObj& get() const
		{
			if (CObjIDTrait<CObj>::ID == _id)
				return *_obj.cast<CObj>();
			else
				throw Error(ErrorCode::INVALID_TYPE);
		}

		template<typename CObj>
		CObj& set()
		{
			if (CObjIDTrait<CObj>::ID == _id)
				return *_obj.cast<CObj>();
			else
				throw Error(ErrorCode::INVALID_TYPE);
		}
	};
}
