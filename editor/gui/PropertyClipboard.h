#pragma once

#include <vector>

namespace oly::editor
{
	typedef unsigned int PropUID;

	namespace prop
	{
		extern PropUID _prop_uid_counter;

#define OLY_DECL_PROP_UID (_prop_uid_counter++)

		extern const PropUID NULL_UID;
	}

	struct RawPropertyPayload
	{
		std::vector<std::byte> data;
		PropUID type;

		RawPropertyPayload();

		bool Empty() const;

		static RawPropertyPayload Make(const void* stack_payload, size_t size, PropUID type);

		template<typename T>
		static RawPropertyPayload Make(const T& stack_payload, PropUID type)
		{
			return Make(&stack_payload, sizeof(T), type);
		}
	};

	struct IPropertyView
	{
		virtual ~IPropertyView() = default;
		virtual RawPropertyPayload Dump() const = 0;
		virtual bool CanParse(const RawPropertyPayload&) const = 0;
		virtual bool TryParse(const RawPropertyPayload&) = 0;
	};

	// TODO v9.1 primitive IPropertyView subclasses: IntPayload, FloatPayload (can parse IntPayload or FloatPayload, etc.), etc.
	// TODO v9.1 some kind of aggregate payload system for descriptors/subforms -> dump std::vector<RawPropertyPayload>

	struct PropertyClipboard
	{
		static void Clear();
		static void Store(const IPropertyView& prop);
		static bool CanPaste(const IPropertyView& prop);
		static bool TryPaste(IPropertyView& prop);

		static bool ContextMenuItems(IPropertyView& prop);
	};
}
