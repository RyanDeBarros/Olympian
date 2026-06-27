#pragma once

#include "documents/IDocument.h"

#include "desc/impl/RasterFontDesc.h"

#include "gui/ListModel.h"

#include "assets/MetaSplitter.h"

namespace oly::editor
{
	class RasterFontDocument : public IDocument
	{
		DoubleDescriptor<RasterFontDesc> _desc;
		detail::MetaMap _meta;
		gui::ListModel _glyph_model;
		Counter<std::string> _codepoint_counter;

	public:
		using IDocument::IDocument;

		static const char* GetVersion();

		void InitImpl() override;
		void Draw() override;
		void Load() override;
		void Dump() override;
		const IDoubleDescriptor& GetDoubleDescriptor() const override;
		IDoubleDescriptor& GetDoubleDescriptor() override;

	private:
		void Draw(DataPath path, RasterFontDesc& desc);
		void Draw(DataPath path, GlyphDesc& desc);

		void Load(TOMLNode node, RasterFontDesc& desc);
		void Load(TOMLNode node, GlyphDesc& desc);

		void Dump(toml::table& table, RasterFontDesc& desc);
		void Dump(toml::table& table, GlyphDesc& desc);

		std::unique_ptr<gui::ListCallbackAdapter> ListAdapter();
	};
}
