#pragma once

#include <functional>
#include <memory>
#include <span>
#include <variant>
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

		static RawPropertyPayload Make(const void* data, size_t size, PropUID type);

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
		virtual bool TryParse(const RawPropertyPayload&) const = 0;
	};

	struct PropertyRow
	{
		std::vector<std::unique_ptr<IPropertyView>> list;
	};

	struct PropertyPage
	{
		using V = std::variant<PropertyRow, std::unique_ptr<PropertyPage>>;
		std::vector<V> page;
	};

	using PropertyPageGenerator = std::function<PropertyPage()>;

	struct PropertyClipboard
	{
		static void Clear();

		static bool ContextMenuItems(const IPropertyView& prop);
		static bool ContextMenuItems(const PropertyRow& props);
		static bool ContextMenuItems(const PropertyPage& props);
	};
}
