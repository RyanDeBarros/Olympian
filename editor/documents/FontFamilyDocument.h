#pragma once

#include "documents/IDocument.h"

#include "desc/impl/FontFamilyDesc.h"

#include "gui/scopes/Form.h"

#include "assets/MetaSplitter.h"

namespace oly::editor
{
	class FontFamilyDocument : public IDocument
	{
		FontFamilyDesc _scratch;
		FontFamilyDesc _disk;
		detail::MetaMap _meta;

	public:
		using IDocument::IDocument;

		static const char* GetVersion();

		void Init() override;
		void Draw() override;
		void Load() override;
		void Dump() override;

	private:
		void Draw(FontFamilyDesc& desc);
		void Draw(FontStyleDesc& desc);

		void Load(TOMLNode node, FontFamilyDesc& desc);
		void Load(TOMLNode node, FontStyleDesc& desc);

		void Dump(toml::table& table, FontFamilyDesc& desc);
		void Dump(toml::table& table, FontStyleDesc& desc);

		FontStyleDesc& GetFontStyleDesc(detail::FontStyleMode style);
	};
}
