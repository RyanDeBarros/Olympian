#include "FontDocument.h"

#include "core/editor/Notifier.h"

#include "definitions/Keys.h"

#include "util/Counter.h"
#include "util/Parser.h"
#include "util/DynamicArray.h"

#include <imp/hash.hpp>

namespace oly::editor
{
	const char* FontDocument::GetVersion()
	{
		return "1.0";
	}

	FontDocument::~FontDocument()
	{
		DestroyFont();
	}

	void FontDocument::InitImpl()
	{
		if (!GetSourcePath().is_resource())
			Notifier::NotifyWarning("Asset is not located in resource folder");

		_atlas_slots.policy = gui::ListPolicy::MinimumOne;
		_display_text.value = "Abc 123";
		_display_text.config().label = "Display text";
		LoadAsset();
	}

	void FontDocument::Draw()
	{
		auto pre_draw = PreDraw();

		imtk::id_scope scope(this);

		if (auto _ = imtk::tab_bar(""))
		{
			if (auto _ = imtk::tab_item("Font Face"))
				DrawFontFace();

			if (auto _ = imtk::tab_item("Font Atlases"))
				DrawFontAtlases();
		}
	}

	void FontDocument::LoadImpl()
	{
		if (_oly_path.is_file())
		{
			_meta = detail::MetaSplitter::decode_meta(_oly_path);

			toml::table table;
			std::string err = _oly_path.load_toml(table);
			if (err.empty())
				Load(imtk::toml_node(table), _desc.disk);
			else
				Notifier::NotifyError("cannot load font - corrupted asset: " + GetSourcePath().string());

			MarkClean();
		}
		else
		{
			Load(imtk::toml_node(), _desc.disk);

			_meta = {};
			_meta.map[detail::Key::Meta_Version] = GetVersion();
			_meta.map[detail::Key::Meta_Import] = "1";
			_meta.map[detail::Key::Meta_Type] = detail::encode_key(detail::Key::Meta_Font);

			MarkDirty();
		}

		_desc.LoadFromDisk();

		_atlas_slots.Init(*FontAtlasListAdapter());
	}

	void FontDocument::DumpImpl()
	{
		toml::table table;
		Dump(table, _desc.scratch);
		_oly_path.dump_toml(table, _meta);
		_desc.WriteToDisk();
		MarkClean();
	}

	void FontDocument::ResetAssetImpl()
	{
		Load(imtk::toml_node(), _desc.scratch);
	}

	const IDoubleDescriptor& FontDocument::GetDoubleDescriptor() const
	{
		return _desc;
	}

	IDoubleDescriptor& FontDocument::GetDoubleDescriptor()
	{
		return _desc;
	}

	detail::ResourcePath FontDocument::GetSourcePath() const
	{
		return _oly_path.get_source_path();
	}

	void FontDocument::ReloadFont()
	{
		DestroyFont();
		_preview_font = ImGui::GetIO().Fonts->AddFontFromFileTTF(GetSourcePath().string().c_str(), _desc.scratch.font_atlases[_atlas_slots.active_index].font_size.value);
	}

	void FontDocument::DestroyFont()
	{
		if (_preview_font)
		{
			ImGui::GetIO().Fonts->RemoveFont(_preview_font);
			_preview_font = nullptr;
		}
	}

	void FontDocument::DrawFontFace()
	{
		if (auto form = imtk::prop::form())
			Draw(_desc.scratch.font_face);
	}

	void FontDocument::DrawFontAtlases()
	{
		if (auto _ = imtk::table("", 2))
		{
			ImGui::TableNextColumn();
				
			_atlas_slots.Update(*FontAtlasListAdapter());
			if (auto scope = imtk::id_scope("##Atlas"))
				_atlas_slots.DrawComboHeader({ .prompt = "Select atlas", .create_tooltip = "New atlas", .delete_tooltip = "Delete atlas", .clear_tooltip = "Clear atlases" }, "Atlas");
				
			if (auto form = imtk::prop::form())
			{
				if (!_desc.scratch.font_atlases.empty())
					Draw(_desc.scratch.font_atlases[_atlas_slots.active_index]);

				if (_atlas_slots.ConsumeOps(*FontAtlasListAdapter()))
					MarkDirty();

				if (_atlas_slots.active_index.ConsumeModified())
					DestroyFont();
			}

			ImGui::TableNextColumn();
			DrawAtlasPreview();
		}
	}

	void FontDocument::DrawAtlasPreview()
	{
		if (auto _ = imtk::child("Preview", ImVec2(0, 0), ImGuiChildFlags_Borders))
		{
			ImGui::TextUnformatted("Preview");
			ImGui::Separator();

			_display_text.draw();

			if (!_preview_font)
				ReloadFont();

			if (auto _ = imtk::font_scope(_preview_font))
				ImGui::TextUnformatted(_display_text.value.c_str());
		}
	}

	void FontDocument::Draw(FontFaceDesc& desc)
	{
		desc.storage.draw();

		struct CodepointHash
		{
			size_t operator()(const std::string& str) const
			{
				if (auto v = stocdpt(str))
					return std::hash<int>{}(*v);
				else
					return 0;
			}
		};

		struct CodepointPairEquality
		{
			bool operator()(const std::array<std::string, 2>& lhs, const std::array<std::string, 2>& rhs) const
			{
				return stocdpt(lhs[0]) == stocdpt(rhs[0]) && stocdpt(lhs[1]) == stocdpt(rhs[1]);
			}
		};

		Counter<std::array<std::string, 2>, imp::stl_hash<CodepointHash>, CodepointPairEquality> counter;
		for (auto& k : desc.kerning)
		{
			k.distance.edit.pre_edit();
			k.pair.edits[0].pre_edit();
			k.pair.edits[1].pre_edit();
			counter.increment({ k.pair.edits[0].buffer(), k.pair.edits[1].buffer() });
		}

		for (size_t i = 0; i < desc.kerning.size(); ++i)
		{
			auto& k = desc.kerning[i];
			if (k.distance.edit.buffer() != k.distance.def || k.pair.edits[0].buffer() != k.pair.def[0] || k.pair.edits[1].buffer() != k.pair.def[1])
				imtk::prop::reset::button(1 + i);
		}

		DescIO::DrawDynamicList(desc.kerning.link, "Kerning", desc.kerning, {}, [&desc, &counter](gui::DynamicRow& row) -> imtk::item_result {
			imtk::w::widget_row components;
			auto& k = desc.kerning[row.Index()];

			bool dup_warning = counter.count({ k.pair.edits[0].buffer(), k.pair.edits[1].buffer() }) > 1;
			imtk::outline dup_outline;
			for (size_t i = 0; i < 2; ++i)
			{
				components.subwidgets.push_back(std::make_unique<imtk::w::generic_widget>([&k, i, &dup_warning, &dup_outline]() -> imtk::item_result {
					bool bad_codepoint = !stocdpt(k.pair.edits[i].buffer()).has_value();
					imtk::outline bad_outline;
					if (bad_codepoint)
						dup_warning = false;

					imtk::item_result result;

					if (i == 0)
					{
						ImGui::TextUnformatted(k.pair.label);
						result |= imtk::item_result::query(false);
						ImGui::SameLine();
					}

					result |= imtk::w::bound_widget<std::string>(k.pair.edits[i].buffer(), { .label = k.pair.sublabels ? k.pair.sublabels[i] : "" }).draw();
					k.pair.edits[i].post_edit(result.state);

					if (dup_warning && result.state.hovered())
						ImGui::SetTooltip("Duplicate codepoint pair");

					if (bad_codepoint)
					{
						if (result.state.hovered())
							ImGui::SetTooltip("Bad codepoint format");

						bad_outline.draw(imtk::col::error);
					}

					if (i == 1)
					{
						if (dup_warning)
							dup_outline.draw(imtk::col::error);
					}

					return result;
				}));
			}

			components.subwidgets.push_back(std::make_unique<imtk::w::generic_widget>([&k]() -> imtk::item_result {
				imtk::controls::vertical_separator();
				ImGui::TextUnformatted(k.distance.label);
				auto result = imtk::item_result::query(false);
				ImGui::SameLine();
				result |= imtk::w::bound_widget<int>(k.distance.edit.buffer()).draw();
				k.distance.edit.post_edit(result.state);
				return result;
			}));

			return components.draw();
		}, desc.kerning_ui_state);

		for (size_t i = 0; i < desc.kerning.size(); ++i)
		{
			KerningDesc& k = desc.kerning[i];
			if (imtk::prop::reset::activated(1 + i))
			{
				k.distance.edit.publish_reset(k.distance.def);
				k.pair.edits[0].publish_reset(k.pair.def[0]);
				k.pair.edits[1].publish_reset(k.pair.def[1]);
				MarkDirty();
			}

			bool publish_action = false;
			publish_action |= k.distance.edit.consume_modified();
			publish_action |= k.pair.edits[0].consume_modified();
			publish_action |= k.pair.edits[1].consume_modified();
			if (publish_action)
			{
				KerningDesc original;
				original.distance.value = std::move(k.distance.edit.original());
				original.pair.value[0] = std::move(k.pair.edits[0].original());
				original.pair.value[1] = std::move(k.pair.edits[1].original());
				PushDescriptorSetAction(k.link.compute_path(), std::move(original), imtk::desc::clone_data(k));
			}
		}
	}
	
	void FontDocument::Draw(FontAtlasDesc& desc)
	{
		desc.font_size.draw();
		if (imtk::prop::row::dirty())
			DestroyFont();

		IMTK_DRAW_FIELDS(FONT_ATLAS_NONPREVIEW_GENERATOR);

		if (auto subform = imtk::prop::subform("Common buffer"))
		{
			desc.use_common_buffer_preset.draw();
			bool preset = desc.use_common_buffer_preset.value;
			
			if (auto d = imtk::disabled(!preset))
			{
				desc.common_buffer_preset.draw();
				if (auto scope = imtk::id_scope(&desc.common_buffer_preset))
				{
					imtk::prop::value::add_component(std::make_unique<imtk::w::readonly_text_owned>(detail::buffer_of(desc.common_buffer_preset.value)));
					imtk::prop::row::submit();
				}
			}

			if (auto d = imtk::disabled(preset))
				desc.common_buffer.draw();
		}
	}

	void FontDocument::Load(imtk::toml_node node, FullFontDesc& desc)
	{
		Load(node[detail::encode_key(desc.font_face_key)], desc.font_face);

		const toml::array* array = node[detail::encode_key(desc.font_atlas_key)].as_array();
		if (array && !array->empty())
		{
			for (size_t i = 0; i < array->size(); ++i)
			{
				desc.font_atlases.push_back();
				Load(imtk::toml_node(*array->get(i)), desc.font_atlases.back());
			}
		}
		else
		{
			desc.font_atlases.push_back();
			Load(imtk::toml_node(), desc.font_atlases.back());
		}
	}

	void FontDocument::Load(imtk::toml_node node, FontFaceDesc& desc)
	{
		desc.storage.load(node);

		const toml::array* array = node[detail::encode_key(desc.kerning_key)].as_array();
		if (array && !array->empty())
		{
			for (size_t i = 0; i < array->size(); ++i)
			{
				desc.kerning.push_back();
				Load(imtk::toml_node(*array->get(i)), desc.kerning.back());
			}
		}
	}

	void FontDocument::Load(imtk::toml_node node, KerningDesc& desc)
	{
		IMTK_LOAD_FIELDS(KERNING_GENERATOR);
	}

	void FontDocument::Load(imtk::toml_node node, FontAtlasDesc& desc)
	{
		IMTK_LOAD_FIELDS(FONT_ATLAS_GENERATOR);
	}

	void FontDocument::Dump(toml::table& table, FullFontDesc& desc)
	{
		toml::table subtable;
		Dump(subtable, desc.font_face);
		table.insert_or_assign(detail::encode_key(desc.font_face_key), std::move(subtable));

		toml::array array;
		for (auto& d : desc.font_atlases)
			Dump(array.emplace_back<toml::table>(), d);
		table.insert_or_assign(detail::encode_key(desc.font_atlas_key), std::move(array));
	}

	void FontDocument::Dump(toml::table& table, FontFaceDesc& desc)
	{
		desc.storage.dump(table);
		
		toml::array array;
		for (auto& d : desc.kerning)
			Dump(array.emplace_back<toml::table>(), d);
		table.insert_or_assign(detail::encode_key(desc.kerning_key), std::move(array));
	}

	void FontDocument::Dump(toml::table& table, KerningDesc& desc)
	{
		IMTK_DUMP_FIELDS(KERNING_GENERATOR);
	}

	void FontDocument::Dump(toml::table& table, FontAtlasDesc& desc)
	{
		IMTK_DUMP_FIELDS(FONT_ATLAS_GENERATOR);
	}

	struct BriefDescPrinter
	{
		void operator()(std::ostream& os, const FontAtlasDesc& desc) const
		{
			os << "FontAtlasDesc[font_size=" << desc.font_size.value << ", ...]";
		}
	};

	std::unique_ptr<gui::IListAdapter> FontDocument::FontAtlasListAdapter()
	{
		return gui::MakeVectorAdapter<BriefDescPrinter>(_desc.scratch.font_atlases);
	}
}
