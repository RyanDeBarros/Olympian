#include "PropertyPayloads.h"

#include "core/Types.h"

#include <string>

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
	bool CanParsePropertyPayload<Type>(const RawPropertyPayload& payload) \
	{ \
		return payload.type == UID; \
	} \
	template<> \
	bool TryParsePropertyPayload(const RawPropertyPayload& payload, Type& value) \
	{ \
		if (payload.type == UID) \
		{ \
			value = *reinterpret_cast<const Type*>(payload.data.data()); \
			return true; \
		} \
		else \
			return false; \
	}

	PRIMITIVE_PAYLOAD(bool, BOOL_UID);
	PRIMITIVE_PAYLOAD(int, INT_UID);
	PRIMITIVE_PAYLOAD(float, FLOAT_UID);
	PRIMITIVE_PAYLOAD(double, DOUBLE_UID);
	PRIMITIVE_PAYLOAD(glm::vec2, VEC2_UID);
	PRIMITIVE_PAYLOAD(glm::vec3, VEC3_UID);
	PRIMITIVE_PAYLOAD(glm::vec4, VEC4_UID);
	PRIMITIVE_PAYLOAD(Color4, COLOR4_UID);

	static PropUID STRING_UID = OLY_DECL_PROP_UID;

	template<>
	RawPropertyPayload MakePropertyPayload(const std::string& value)
	{
		return RawPropertyPayload::Make(value.data(), value.size(), STRING_UID);
	}

	template<>
	bool CanParsePropertyPayload<std::string>(const RawPropertyPayload& payload)
	{
		return payload.type == STRING_UID;
	}

	template<>
	bool TryParsePropertyPayload(const RawPropertyPayload& payload, std::string& value)
	{
		if (payload.type == STRING_UID)
		{
			value = std::string(reinterpret_cast<const char*>(payload.data.data()), payload.data.size());
			return true;
		}
		else
			return false;
	}
}
