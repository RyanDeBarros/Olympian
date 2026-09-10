#include "FontDocument.h"

#include "assets/TranslateKey.h"
#include "definitions/Keys.h"

#include <imp/counter.hpp>
#include <imp/equal.hpp>
#include <imp/hash.hpp>
#include <imp/parser.hpp>

namespace oly::editor
{
	FontDocument::FontDocument(detail::ResourcePath oly_path)
		: IDocument(std::move(oly_path))
		, _atlas_slots({ .prompt = "Select atlas", .create_tooltip = "Create atlas", .delete_tooltip = "Delete atlas", .clear_tooltip = "Clear atlases" }, "Atlas")
	{
	}

	const char* FontDocument::GetVersion()
	{
		return "1.0";
	}

	void FontDocument::InitImpl()
	{
		if (!GetSourcePath().is_resource())
			imtk::notify_warning("Asset is not located in resource folder");

		_atlas_slots.model.policy = imtk::list_policy::minimum_one;
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
				imtk::notify_error("cannot load font - corrupted asset: " + GetSourcePath().string());

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

		_desc.load_from_disk();

		_atlas_slots.model.init(*FontAtlasListAdapter());
	}

	void FontDocument::DumpImpl()
	{
		toml::table table;
		Dump(table, _desc.scratch);
		_oly_path.dump_toml(table, _meta);
		_desc.write_to_disk();
		MarkClean();
	}

	void FontDocument::ResetAssetImpl()
	{
		Load(imtk::toml_node(), _desc.scratch);
	}

	const imtk::desc::idoubler& FontDocument::GetDoubleDescriptor() const
	{
		return _desc;
	}

	imtk::desc::idoubler& FontDocument::GetDoubleDescriptor()
	{
		return _desc;
	}

	detail::ResourcePath FontDocument::GetSourcePath() const
	{
		return _oly_path.get_source_path();
	}

	void FontDocument::DrawFontFace()
	{
		if (auto form = imtk::prop::form())
			Draw(*_desc.scratch.font_face);
	}

	void FontDocument::DrawFontAtlases()
	{
		if (auto _ = imtk::table("", 2))
		{
			ImGui::TableNextColumn();
				
			_atlas_slots.model.sync(*FontAtlasListAdapter());
			if (auto scope = imtk::id_scope("##Atlas"))
				_atlas_slots.draw();
				
			if (auto form = imtk::prop::form())
			{
				if (!_desc.scratch.font_atlases.empty())
					Draw(_desc.scratch.font_atlases[_atlas_slots.model.index()]);

				if (_atlas_slots.model.consume_ops(*FontAtlasListAdapter()))
					MarkDirty();

				if (_atlas_slots.model.consume_index_modified())
					_preview_font.reset();
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
				_preview_font = imtk::font_instance(GetSourcePath().string().c_str(), _desc.scratch.font_atlases[_atlas_slots.model.index()].font_size.value);

			if (auto _ = imtk::font_scope(_preview_font))
				ImGui::TextUnformatted(_display_text.value.c_str());
		}
	}

	void FontDocument::Draw(FontFaceDesc& desc)
	{
		desc.storage.draw();

		imp::counter<std::array<std::string, 2>, imp::stl_hash<imp::cdpt_hash>, imp::stl_equal<imp::cdpt_equal>> counter;
		for (auto& k : desc.kerning)
			counter.increment({ k.pair.fields[0].edit.buffer(), k.pair.fields[1].edit.buffer() });

		// TODO v9.3 put in init
		desc.kerning_widget.body.row_draw = [&desc, &counter](imtk::w::dynamic_row& row) -> imtk::item_result {
			imtk::w::widget_row components;
			auto& k = desc.kerning[row.index()];

			bool dup_warning = counter.count({ k.pair.fields[0].edit.buffer(), k.pair.fields[1].edit.buffer() }) > 1;
			imtk::outline dup_outline;
			for (size_t i = 0; i < 2; ++i)
			{
				components.subwidgets.push_back(std::make_unique<imtk::w::generic_widget>([&k, i, &dup_warning, &dup_outline]() -> imtk::item_result {
					bool bad_codepoint = !imp::stocdpt(k.pair.fields[i].edit.buffer()).has_value();
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

					result |= imtk::w::bound_widget<std::string>(k.pair.fields[i].edit.buffer(), { .label = "" }).draw();

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
				return result;
			}));

			auto result = components.draw();
			k.pair.fields[0].edit.post_edit(result.state);
			k.pair.fields[1].edit.post_edit(result.state);
			k.distance.edit.post_edit(result.state);
			return result;
		};

		std::vector<std::unique_ptr<imtk::prop::iresettable>> resetters;
		resetters.push_back(std::make_unique<imtk::prop::resettable_vector_size<KerningDesc>>(desc.kerning_widget.model, desc.kerning, 0));

		for (auto& k : desc.kerning)
		{
			// TODO v9.3 inline complex field support - instead of PrimitiveField, InlineComplexField?

			resetters.push_back(imtk::prop::make_resettable_value_row(
				imtk::prop::make_resettable_value(k.distance.edit, k.distance.def),
				imtk::prop::make_resettable_value(k.pair.fields[0].edit, k.pair.fields[0].def),
				imtk::prop::make_resettable_value(k.pair.fields[1].edit, k.pair.fields[1].def)
			));
		}

		if (auto _ = imtk::prop::multi_row_scope("Kerning", std::move(resetters)))
			imtk::prop::value::add_component(std::make_unique<imtk::w::generic_widget>([&desc]() { return desc.kerning_widget.draw(desc.kerning.size()); }));

		for (size_t i = 0; i < desc.kerning.size(); ++i)
		{
			KerningDesc& k = desc.kerning[i];
			auto og_distance = k.distance.edit.consume_published_from();
			auto og_pair_0 = k.pair.fields[0].edit.consume_published_from();
			auto og_pair_1 = k.pair.fields[1].edit.consume_published_from();
			if (og_distance || og_pair_0 || og_pair_1)
			{
				KerningDesc original;
				original.distance.value = og_distance.value_or(k.distance.value);
				original.pair.fields[0].value = og_pair_0.value_or(k.pair.fields[0].value);
				original.pair.fields[1].value = og_pair_1.value_or(k.pair.fields[1].value);
				imtk::desc::push_set_action(k.link.compute_path(), std::move(original), imtk::desc::clone_data(k));
			}
		}

		if (desc.kerning_widget.model.visit_deferred_ops([&desc](const imtk::list_op& op) { op.execute_desc_action<KerningDesc>(desc.kerning.link.compute_path()); }))
			imtk::prop::grid::mark_dirty();
	}
	
	void FontDocument::Draw(FontAtlasDesc& desc)
	{
		desc.font_size.draw();
		if (imtk::prop::row::dirty())
			_preview_font.reset();

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
		Load(desc.font_face.subnode(node), *desc.font_face);

		const toml::array* array = desc.font_atlases.subnode(node).as_array();
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

		const toml::array* array = desc.kerning.subnode(node).as_array();
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
		Dump(subtable, *desc.font_face);
		desc.font_face.dump_into(table, std::move(subtable));

		toml::array array;
		for (auto& d : desc.font_atlases)
			Dump(array.emplace_back<toml::table>(), d);
		desc.font_atlases.dump_into(table, std::move(array));
	}

	void FontDocument::Dump(toml::table& table, FontFaceDesc& desc)
	{
		desc.storage.dump(table);
		
		toml::array array;
		for (auto& d : desc.kerning)
			Dump(array.emplace_back<toml::table>(), d);
		desc.kerning.dump_into(table, std::move(array));
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

	std::unique_ptr<imtk::list_adapter> FontDocument::FontAtlasListAdapter() const
	{
		return imtk::make_vector_adapter<BriefDescPrinter>(_desc.scratch.font_atlases);
	}
}
