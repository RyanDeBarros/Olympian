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

	// TODO v7 make this a general utility for documents
	static const char* ComboGetter(void* data, int idx)
	{
		auto& items = *static_cast<std::vector<std::string>*>(data);
		if (idx < 0 || idx >= items.size())
			return nullptr;
		else
			return items[idx].c_str();
	}

	void TextureDocument::Draw(TextureDescVariant& desc)
	{
		if (DescIO::BeginForm(&desc))
		{
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::Text("Select Slot");

			ImGui::SameLine();
			if (ImGui::Button("+"))
			{
				_active_slot = _slot_names.size();
				_scratch.PushBack();
				MarkDirty();
			}

			ImGui::SameLine();
			if (ImGui::Button("-"))
			{
				_scratch.Remove(_active_slot);
				if (_scratch.Empty())
					_scratch.PushBack();
				MarkDirty();
			}

			GenSlotNames();
			ImGui::TableNextColumn();
			ImGui::Combo("##SelectSlot", &_active_slot, ComboGetter, &_slot_names, _slot_names.size());

			desc.Visit(_active_slot, [this](auto& d) { Draw(d); });
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

	void TextureDocument::Load(TOMLNode node, TextureDescVariant& desc)
	{
		const auto Clear = [this](TextureDescVariant& desc) {
			if (_svg)
				desc.Clear<VectorTextureDesc>();
			else
				desc.Clear<RasterTextureDesc>();
		};

		TOMLArray array = node[detail::encode_key(detail::Key::TextureArray)].as_array();
		if (array && !array->empty())
		{
			Clear(desc);
			for (size_t i = 0; i < array->size(); ++i)
				desc.PushBack();

			desc.VisitIndexed([this, &array](size_t i, auto& d) { Load(TOMLNode(*array->get(i)), d); });
		}
		else
		{
			Clear(desc);
			desc.PushBack();

			desc.Visit(0, [this](auto& d) { Load(TOMLNode(), d); });
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

	void TextureDocument::Dump(toml::table& table, TextureDescVariant& desc)
	{
		toml::v3::array array;
		desc.Visit([this, &array](auto& d) {
			toml::table table;
			Dump(table, d);
			array.push_back(std::move(table));
		});
		table.insert_or_assign(detail::encode_key(detail::Key::TextureArray), std::move(array));
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

	void TextureDocument::GenSlotNames()
	{
		_slot_names.clear();
		for (int i = 0; i < _scratch.Count(); ++i)
			_slot_names.push_back("Slot " + std::to_string(i));

		if (_scratch.Empty())
			_active_slot = 0;
		else if (_active_slot >= _scratch.Count())
			_active_slot = _scratch.Count() - 1;
	}
}
