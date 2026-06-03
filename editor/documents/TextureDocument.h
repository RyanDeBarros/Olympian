#pragma once

#include "documents/IDocument.h"
#include "graphics/Texture.h"

#include "desc/TextureDesc.h"

#include "assets/MetaSplitter.h"

namespace oly::editor
{
	struct PreviewNav
	{
		ImVec2 pos = ImVec2(0, 0);
		float zoom = 0.f;
	};

	class TextureDocument : public IDocument
	{
		TextureDescVariant _scratch;
		TextureDescVariant _disk;
		detail::MetaMap _meta;
		bool _gif = false;
		bool _svg = false;
		int _active_slot = 0;
		std::vector<std::string> _slot_names;
		Texture _texture;
		PreviewNav _preview_nav;

	public:
		using IDocument::IDocument;

		static const char* GetVersion();

		void Init() override;
		void Draw() override;
		void Load() override;
		void Dump() override;

		detail::ResourcePath GetSourcePath() const;

	private:
		void DrawPreview();
		
		void Draw(TextureDescVariant& desc);
		void Draw(RasterTextureDesc& desc);
		void Draw(VectorTextureDesc& desc);
		void Draw(BaseTextureDesc& desc);
		void Draw(SpritesheetDesc& desc);

		void Load(TOMLNode node, TextureDescVariant& desc);
		void Load(TOMLNode node, RasterTextureDesc& desc);
		void Load(TOMLNode node, VectorTextureDesc& desc);
		void Load(TOMLNode node, BaseTextureDesc& desc);
		void Load(TOMLNode node, SpritesheetDesc& desc);

		void Dump(toml::table& table, TextureDescVariant& desc);
		void Dump(toml::table& table, RasterTextureDesc& desc);
		void Dump(toml::table& table, VectorTextureDesc& desc);
		void Dump(toml::table& table, BaseTextureDesc& desc);
		void Dump(toml::table& table, SpritesheetDesc& desc);

		void GenSlotNames();
	};
}
