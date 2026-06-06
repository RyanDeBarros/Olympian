#pragma once

#include "documents/IDocument.h"
#include "graphics/Form.h"
#include "graphics/Texture.h"

#include "desc/TextureDesc.h"

#include "assets/MetaSplitter.h"

namespace oly::editor
{
	struct PreviewNav
	{
		ImVec2 pos = ImVec2(0, 0);
		float zoom = 0.f;
		float svg_scale = 1.f;
	};

	struct SpritesheetPreviewData
	{
		int active_index = 0;
		float timer = 0.f;
		bool playing = false;
	};

	struct SpritesheetInfo
	{
		int rows, cols;
		float cell_width, cell_height;
		float full_width, full_height;
		ImVec2 rect_offset;
	};

	class TextureDocument : public IDocument
	{
		TextureVariantDesc _scratch;
		TextureVariantDesc _disk;
		detail::MetaMap _meta;
		bool _gif = false;
		bool _svg = false;
		int _active_slot = 0;
		std::vector<std::string> _slot_names;
		Texture _texture;
		PreviewNav _preview_nav;
		bool _preview_spritesheet = true;
		SpritesheetPreviewData _spritesheet_preview_data;

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
		void DrawPreview();
		SpritesheetDesc* SpritesheetPreview();
		SpritesheetInfo CalcSpritesheetInfo(const SpritesheetDesc& desc);
		void DrawSpritesheetOverlay(const SpritesheetDesc& desc, ImVec2 rect_start, ImVec2 size);
		void PlaySpritesheetAnimation(const SpritesheetDesc& desc);
		
		void Draw(TextureVariantDesc& desc);
		void Draw(Form& form, RasterTextureDesc& desc);
		void Draw(Form& form, VectorTextureDesc& desc);
		void Draw(Form& form, BaseTextureDesc& desc);
		void Draw(Form& form, SpritesheetDesc& desc);

		void Load(TOMLNode node, TextureVariantDesc& desc);
		void Load(TOMLNode node, RasterTextureDesc& desc);
		void Load(TOMLNode node, VectorTextureDesc& desc);
		void Load(TOMLNode node, BaseTextureDesc& desc);
		void Load(TOMLNode node, SpritesheetDesc& desc);

		void Dump(toml::table& table, TextureVariantDesc& desc);
		void Dump(toml::table& table, RasterTextureDesc& desc);
		void Dump(toml::table& table, VectorTextureDesc& desc);
		void Dump(toml::table& table, BaseTextureDesc& desc);
		void Dump(toml::table& table, SpritesheetDesc& desc);

		void GenSlotNames();
	};
}
