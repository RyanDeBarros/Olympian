#pragma once

#include "documents/IDocument.h"

#include "desc/impl/FontDesc.h"
#include "desc/DoubleDescriptor.h"

#include "gui/ListModel.h"

#include "assets/MetaSplitter.h"

namespace oly::editor
{
	class FontDocument : public IDocument
	{
		DoubleDescriptor<FullFontDesc> _desc;
		detail::MetaMap _meta;
		gui::ListModel _atlas_slots;
		std::string _display_text;
		ImFont* _preview_font = nullptr;
		
	public:
		using IDocument::IDocument;
		~FontDocument();

		static const char* GetVersion();

		void InitImpl() override;
		void Draw() override;
		void LoadImpl() override;
		void DumpImpl() override;
		void ResetAssetImpl() override;
		const IDoubleDescriptor& GetDoubleDescriptor() const override;
		IDoubleDescriptor& GetDoubleDescriptor() override;

		detail::ResourcePath GetSourcePath() const;

	private:
		void ReloadFont();
		void DestroyFont();

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

		std::unique_ptr<gui::IListAdapter> FontAtlasListAdapter();
	};
}
