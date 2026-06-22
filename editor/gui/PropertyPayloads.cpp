#include "PropertyPayloads.h"

namespace oly::editor::prop
{
#define PRIMITIVE_PAYLOAD(Type, UID) \
	static PropUID UID = OLY_DECL_PROP_UID; \
	template<> \
	RawPropertyPayload MakePropertyPayload(const Type& value) \
	{ \
		return RawPropertyPayload::Make(value, UID); \
	} \
	template<> \
	std::optional<Type> ParsePropertyPayload(const RawPropertyPayload& payload) \
	{ \
		if (payload.type == UID) \
			return *reinterpret_cast<const Type*>(payload.data.data()); \
		else \
			return std::nullopt; \
	}

	PRIMITIVE_PAYLOAD(bool, BOOL_UID);
	PRIMITIVE_PAYLOAD(int, INT_UID);
	PRIMITIVE_PAYLOAD(float, FLOAT_UID);
}
