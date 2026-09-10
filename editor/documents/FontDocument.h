#pragma once

#include "documents/IDocument.h"

#include "desc/FontDesc.h"

#include "assets/MetaSplitter.h"

namespace oly::editor
{
	class FontDocument : public IDocument
	{
		imtk::desc::doubler<FullFontDesc> _desc;
		detail::MetaMap _meta;
		imtk::w::owned_list_indexer _atlas_slots;
		imtk::w::simple_widget<std::string> _display_text;
		imtk::font_instance _preview_font;
		
	public:
		FontDocument(detail::ResourcePath oly_path);

		static const char* GetVersion();

		void InitImpl() override;
		void Draw() override;
		void LoadImpl() override;
		void DumpImpl() override;
		void ResetAssetImpl() override;
		const imtk::desc::idoubler& GetDoubleDescriptor() const override;
		imtk::desc::idoubler& GetDoubleDescriptor() override;

		detail::ResourcePath GetSourcePath() const;

	private:
		void DrawFontFace();
		void DrawFontAtlases();
		void DrawAtlasPreview();
		
		void Draw(FontFaceDesc& desc);
		void Draw(FontAtlasDesc& desc);

		void Load(imtk::toml_node node, FullFontDesc& desc);
		void Load(imtk::toml_node node, FontFaceDesc& desc);
		void Load(imtk::toml_node node, KerningDesc& desc);
		void Load(imtk::toml_node node, FontAtlasDesc& desc);

		void Dump(toml::table& table, FullFontDesc& desc);
		void Dump(toml::table& table, FontFaceDesc& desc);
		void Dump(toml::table& table, KerningDesc& desc);
		void Dump(toml::table& table, FontAtlasDesc& desc);

		std::unique_ptr<imtk::list_adapter> FontAtlasListAdapter() const;
	};
}
