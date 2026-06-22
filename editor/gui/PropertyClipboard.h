#pragma once

#include <vector>

namespace oly::editor
{
	enum class UID : int;

	struct RawPropertyPayload
	{
		std::vector<std::byte> data;
		UID type;

		RawPropertyPayload();

		bool Empty() const;

		static RawPropertyPayload Make(const void* stack_payload, size_t size, UID type);

		template<typename T>
		static RawPropertyPayload Make(const T& stack_payload, UID type)
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
	// TODO v9.1 some kind of aggregate payload system for descriptors/subforms -> store std::vector<std::unique_ptr<IPropertyView>> and dump std::vector<RawPropertyPayload>

	struct PropertyClipboard
	{
		static void Clear();
		static void Store(const IPropertyView& prop);  // TODO v9.1 call from copy context menu
		static bool CanPaste(const IPropertyView& prop);  // TODO v9.1 enable/disable 'paste' option in context menu
		static bool TryPaste(IPropertyView& prop);  // TODO v9.1 call from paste context menu -> if return true, mark dirty
	};
}
