#pragma once

#include "physics/collision/methods/CollisionInfo.h"
#include "physics/collision/objects/Combinations.h"

#include "core/math/Shapes.h"
#include "core/containers/BlackBox.h"

namespace oly::col2d::internal
{
	enum CObjID : unsigned int
	{
		TPRIMITIVE,
		TCOMPOUND,
		TBVH_AABB,
		TBVH_OBB,
		TBVH_KDOP2,
		TBVH_KDOP3,
		TBVH_KDOP4,
		_COUNT
	};

	template<typename T>
	struct CObjIDTrait;

	template<>
	struct CObjIDTrait<TPrimitive>
	{
		static constexpr CObjID ID = CObjID::TPRIMITIVE;
	};

	template<>
	struct CObjIDTrait<TCompound>
	{
		static constexpr CObjID ID = CObjID::TCOMPOUND;
	};

	template<>
	struct CObjIDTrait<TBVH<AABB>>
	{
		static constexpr CObjID ID = CObjID::TBVH_AABB;
	};

	template<>
	struct CObjIDTrait<TBVH<OBB>>
	{
		static constexpr CObjID ID = CObjID::TBVH_OBB;
	};

	template<>
	struct CObjIDTrait<TBVH<KDOP2>>
	{
		static constexpr CObjID ID = CObjID::TBVH_KDOP2;
	};

	template<>
	struct CObjIDTrait<TBVH<KDOP3>>
	{
		static constexpr CObjID ID = CObjID::TBVH_KDOP3;
	};

	template<>
	struct CObjIDTrait<TBVH<KDOP4>>
	{
		static constexpr CObjID ID = CObjID::TBVH_KDOP4;
	};

	template<typename T>
	concept IsColliderObject = requires { { CObjIDTrait<T>::ID } -> std::convertible_to<CObjID>; };

	class ColliderObject
	{
		BlackBox<true> _obj;
		CObjID _id;

	public:
		template<typename CObj, typename = std::enable_if_t<internal::IsColliderObject<std::decay_t<CObj>>>>
		ColliderObject(CObj&& obj) : _obj(std::forward<CObj>(obj)), _id(CObjIDTrait<std::decay_t<CObj>>::ID) {}

		ColliderObject() : _obj(TPrimitive()), _id(CObjIDTrait<TPrimitive>::ID) {}
		ColliderObject(const ColliderObject& other) : _obj(other._obj), _id(other._id) {}
		ColliderObject(ColliderObject&& other) noexcept : _obj(std::move(other._obj)), _id(other._id) {}
		ColliderObject& operator=(const ColliderObject& other) { if (this != &other) { _obj = other._obj; _id = other._id; } return *this; }
		ColliderObject& operator=(ColliderObject&& other) noexcept { if (this != &other) { _obj = std::move(other._obj); _id = other._id; } return *this; }

		const void* raw_obj() const { return _obj.raw(); }
		CObjID id() const { return _id; }

		template<typename CObj, typename = std::enable_if_t<internal::IsColliderObject<std::decay_t<CObj>>>>
		const CObj& get() const
		{
			if (CObjIDTrait<CObj>::ID == _id)
				return *_obj.cast<CObj>();
			else
				throw Error(ErrorCode::INVALID_TYPE);
		}

		template<typename CObj, typename = std::enable_if_t<internal::IsColliderObject<std::decay_t<CObj>>>>
		CObj& set()
		{
			if (CObjIDTrait<CObj>::ID == _id)
				return *_obj.cast<CObj>();
			else
				throw Error(ErrorCode::INVALID_TYPE);
		}
	};
}
