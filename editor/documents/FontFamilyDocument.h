#pragma once

#include "documents/IDocument.h"

#include "desc/FontFamilyDesc.h"

#include "gui/Form.h"

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
		void Load(TOMLNode node, FontFamilyDesc& desc);

		void Dump(toml::table& table, FontFamilyDesc& desc);
	};
}
