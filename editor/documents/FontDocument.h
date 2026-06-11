#pragma once

#include "documents/IDocument.h"
#include "gui/Form.h"

#include "desc/FontDesc.h"

#include "assets/MetaSplitter.h"

namespace oly::editor
{
	class FontDocument : public IDocument
	{
		FullFontDesc _scratch;
		FullFontDesc _disk;
		detail::MetaMap _meta;
		gui::ListModel _atlas_slots;
		
	public:
		using IDocument::IDocument;

		static const char* GetVersion();

		void Init() override;
		void Draw() override;
		void DrawMenuBar() override;
		void Load() override;
		void Dump() override;

		detail::ResourcePath GetSourcePath() const;

	private:
		void DrawFontFace();
		void DrawFontAtlases();

		void Load(TOMLNode node, FullFontDesc& desc);
		void Load(TOMLNode node, FontFaceDesc& desc);
		void Load(TOMLNode node, FontAtlasDesc& desc);

		void Dump(toml::table& table, FullFontDesc& desc);
		void Dump(toml::table& table, FontFaceDesc& desc);
		void Dump(toml::table& table, FontAtlasDesc& desc);
	};
}
