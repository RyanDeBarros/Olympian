#include "TextureDocument.h"

#include "definitions/Keys.h"

#include <imgui.h>

namespace oly::editor
{
	// TODO v7 preview of image on right side of table

	void TextureDocument::Init()
	{
		_gif = GetSourcePath().extension_matches(".gif");
		_svg = GetSourcePath().extension_matches(".svg");
		Load();
	}

	void TextureDocument::Draw()
	{
		Draw(_scratch);
	}

	void TextureDocument::Load()
	{
		if (_oly_path.exists())
		{
			_meta = detail::MetaSplitter::decode_meta(_oly_path);

			toml::table table;
			std::string err = _oly_path.load_toml(table);
			if (err.empty())
				Load(TOMLNode(table), _disk);
			else
			{
				// TODO v7 log error - corrupted asset
			}
		}
		else
		{
			Load(TOMLNode(), _disk);

			_meta = {};
			_meta.map[detail::Key::Meta_Version] = "1.0";
			_meta.map[detail::Key::Meta_Import] = "1";
			_meta.map[detail::Key::Meta_Type] = detail::encode_key(detail::Key::Meta_Texture);
		}

		_scratch.Reset(_disk);
	}

	void TextureDocument::Dump()
	{
		toml::table table;
		Dump(table, _scratch);
		_oly_path.dump_toml(table, _meta);
		_disk.Reset(_scratch);
		MarkClean();
	}

	detail::ResourcePath TextureDocument::GetSourcePath() const
	{
		return _oly_path.get_source_path();
	}

	void TextureDocument::Draw(TextureDesc& desc)
	{
		// TODO v7 combo box to select slot, buttons to create new, delete.
		for (size_t i = 0; i < desc.array.size(); ++i)
			Draw(desc.array[i]);
	}
	
	void TextureDocument::Draw(TextureSlotDesc& desc)
	{
		if (DescIO::BeginForm(&desc))
		{
			std::visit([this](auto& d) { Draw(d); }, desc.variant);
			DescIO::EndForm();
		}
	}
	
	void TextureDocument::Draw(RasterTextureDesc& desc)
	{
		Draw(desc.base);
		DRAW_FIELDS(RASTER_TEXTURE_PARTIAL_GENERATOR);
	}
	
	void TextureDocument::Draw(VectorTextureDesc& desc)
	{
		Draw(desc.base);
		DRAW_FIELDS(VECTOR_TEXTURE_PARTIAL_GENERATOR);
	}
	
	void TextureDocument::Draw(BaseTextureDesc& desc)
	{
		DRAW_FIELDS(TEXTURE_PARAMS_GENERATOR);

		ImGui::BeginDisabled(_gif);
		DRAW_FIELD(anim);
		ImGui::EndDisabled();

		if (desc.anim.scratch)
			Draw(desc.spritesheet);
	}

	void TextureDocument::Draw(SpritesheetDesc& desc)
	{
		DRAW_FIELDS(SPRITESHEET_GENERATOR);
	}

	void TextureDocument::Load(TOMLNode node, TextureDesc& desc)
	{
		desc.array.clear();
		
		TOMLArray array = node[detail::encode_key(detail::Key::TextureArray)].as_array();
		if (array && !array->empty())
		{
			desc.array.resize(array->size());
			
			for (size_t i = 0; i < array->size(); ++i)
				Load(TOMLNode(*array->get(i)), desc.array[i]);
		}
		else
		{
			desc.array.resize(1);
			Load(TOMLNode(), desc.array[0]);
		}
	}
	
	void TextureDocument::Load(TOMLNode node, TextureSlotDesc& desc)
	{
		if (_svg)
		{
			VectorTextureDesc d;
			Load(node, d);
			desc.variant = d;
		}
		else
		{
			RasterTextureDesc d;
			Load(node, d);
			desc.variant = d;
		}
	}
	
	void TextureDocument::Load(TOMLNode node, RasterTextureDesc& desc)
	{
		Load(node, desc.base);
		LOAD_FIELDS(RASTER_TEXTURE_PARTIAL_GENERATOR);
	}
	
	void TextureDocument::Load(TOMLNode node, VectorTextureDesc& desc)
	{
		Load(node, desc.base);
		LOAD_FIELDS(VECTOR_TEXTURE_PARTIAL_GENERATOR);
	}
	
	void TextureDocument::Load(TOMLNode node, BaseTextureDesc& desc)
	{
		LOAD_FIELDS(TEXTURE_PARAMS_GENERATOR);

		if (_gif)
			desc.anim.scratch = true;
		else
			LOAD_FIELD(anim);

		Load(node, desc.spritesheet);
	}

	void TextureDocument::Load(TOMLNode node, SpritesheetDesc& desc)
	{
		LOAD_FIELDS(SPRITESHEET_GENERATOR);
	}

	void TextureDocument::Dump(toml::table& table, TextureDesc& desc)
	{
		for (TextureSlotDesc& d : desc.array)
			Dump(table, d);
	}

	void TextureDocument::Dump(toml::table& table, TextureSlotDesc& desc)
	{
		std::visit([this, &table](auto& d) { Dump(table, d); }, desc.variant);
	}

	void TextureDocument::Dump(toml::table& table, RasterTextureDesc& desc)
	{
		Dump(table, desc.base);
		DUMP_FIELDS(RASTER_TEXTURE_PARTIAL_GENERATOR);
	}

	void TextureDocument::Dump(toml::table& table, VectorTextureDesc& desc)
	{
		Dump(table, desc.base);
		DUMP_FIELDS(VECTOR_TEXTURE_PARTIAL_GENERATOR);
	}

	void TextureDocument::Dump(toml::table& table, BaseTextureDesc& desc)
	{
		DUMP_FIELDS(TEXTURE_PARAMS_GENERATOR);
		DUMP_FIELD(anim);
		if (desc.anim.scratch)
			Dump(table, desc.spritesheet);
	}

	void TextureDocument::Dump(toml::table& table, SpritesheetDesc& desc)
	{
		DUMP_FIELDS(SPRITESHEET_GENERATOR);
	}
}
