#pragma once

#include "documents/IDocument.h"

#include "desc/impl/RasterFontDesc.h"
#include "desc/DoubleDescriptor.h"

#include "gui/ListModel.h"

#include "assets/MetaSplitter.h"

namespace oly::editor
{
	class RasterFontDocument : public IDocument
	{
		DoubleDescriptor<RasterFontDesc> _desc;
		detail::MetaMap _meta;
		gui::ListIndexer _glyphs;
		imp::counter<std::string> _codepoint_counter;

	public:
		RasterFontDocument(detail::ResourcePath oly_path);

		static const char* GetVersion();

		void InitImpl() override;
		void Draw() override;
		void LoadImpl() override;
		void DumpImpl() override;
		void ResetAssetImpl() override;
		const IDoubleDescriptor& GetDoubleDescriptor() const override;
		IDoubleDescriptor& GetDoubleDescriptor() override;

	private:
		void Draw(RasterFontDesc& desc);
		void Draw(GlyphDesc& desc);

		void Load(imtk::toml_node node, RasterFontDesc& desc);
		void Load(imtk::toml_node node, GlyphDesc& desc);

		void Dump(toml::table& table, RasterFontDesc& desc);
		void Dump(toml::table& table, GlyphDesc& desc);

		std::unique_ptr<gui::ListCallbackAdapter> ListAdapter();
	};
}
