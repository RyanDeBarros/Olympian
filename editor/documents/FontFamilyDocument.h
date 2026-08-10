#pragma once

#include "documents/IDocument.h"

#include "desc/impl/FontFamilyDesc.h"
#include "desc/DoubleDescriptor.h"

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
		void LoadImpl() override;
		void DumpImpl() override;
		void ResetAssetImpl() override;
		const IDoubleDescriptor& GetDoubleDescriptor() const override;
		IDoubleDescriptor& GetDoubleDescriptor() override;

	private:
		void Draw(FontFamilyDesc& desc, const char* subform_header, detail::FontStyleMode style);
		void Draw(FontStyleDesc& desc);

		void Load(imtk::toml_node node, FontFamilyDesc& desc);
		void Load(imtk::toml_node node, FontStyleDesc& desc);

		void Dump(toml::table& table, FontFamilyDesc& desc);
		void Dump(toml::table& table, FontStyleDesc& desc);
	};
}
