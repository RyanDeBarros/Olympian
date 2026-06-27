#pragma once

#include "documents/IDocument.h"

#include "desc/impl/FontFamilyDesc.h"

#include "gui/scopes/Form.h"

#include "assets/MetaSplitter.h"

namespace oly::editor
{
	class FontFamilyDocument : public IDocument
	{
		DoubleDescriptor<FontFamilyDesc> _desc;
		detail::MetaMap _meta;

	public:
		using IDocument::IDocument;

		static const char* GetVersion();

		void InitImpl() override;
		void Draw() override;
		void Load() override;
		void Dump() override;
		IDoubleDescriptor& GetDoubleDescriptor() override;

	private:
		void Draw(DataPath path, FontFamilyDesc& desc, const char* subform_header, detail::FontStyleMode style);
		void Draw(DataPath path, FontStyleDesc& desc);

		void Load(TOMLNode node, FontFamilyDesc& desc);
		void Load(TOMLNode node, FontStyleDesc& desc);

		void Dump(toml::table& table, FontFamilyDesc& desc);
		void Dump(toml::table& table, FontStyleDesc& desc);
	};
}
