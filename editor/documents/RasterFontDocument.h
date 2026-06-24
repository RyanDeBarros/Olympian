#pragma once

#include "documents/IDocument.h"

#include "desc/impl/RasterFontDesc.h"

#include "gui/ListModel.h"

#include "assets/MetaSplitter.h"

namespace oly::editor
{
	class RasterFontDocument : public IDocument
	{
		RasterFontDesc _scratch;
		RasterFontDesc _disk;
		detail::MetaMap _meta;
		gui::ListModel _glyph_model;
		Counter<std::string> _codepoint_counter;

	public:
		using IDocument::IDocument;

		static const char* GetVersion();

		void Init() override;
		void Draw() override;
		void Load() override;
		void Dump() override;

	private:
		void Draw(RasterFontDesc& desc);
		void Draw(GlyphDesc& desc);

		void Load(TOMLNode node, RasterFontDesc& desc);
		void Load(TOMLNode node, GlyphDesc& desc);

		void Dump(toml::table& table, RasterFontDesc& desc);
		void Dump(toml::table& table, GlyphDesc& desc);

		std::unique_ptr<gui::ListCallbackAdapter> ListAdapter();
	};
}
