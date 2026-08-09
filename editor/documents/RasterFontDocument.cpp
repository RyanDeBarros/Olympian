#include "RasterFontDocument.h"

#include "core/editor/Notifier.h"
#include "core/Colors.h"

#include "gui/scopes/Form.h"
#include "gui/scopes/Subform.h"
#include "gui/GUIState.h"

#include "definitions/Keys.h"

namespace oly::editor
{
	const char* RasterFontDocument::GetVersion()
	{
		return "1.0";
	}

	void RasterFontDocument::InitImpl()
	{
		if (!GetOlyPath().is_resource())
			Notifier::NotifyWarning("Asset is not located in resource folder");

		LoadAsset();
	}

	void RasterFontDocument::Draw()
	{
		auto pre_draw = PreDraw();

		imtk::id_scope scope(this);
		Draw(_desc.scratch);
	}

	void RasterFontDocument::LoadImpl()
	{
		if (_oly_path.is_file())
		{
			_meta = detail::MetaSplitter::decode_meta(_oly_path);

			toml::table table;
			std::string err = _oly_path.load_toml(table);
			if (err.empty())
				Load(TOMLNode(table), _desc.disk);
			else
				Notifier::NotifyError("cannot load raster font - corrupted asset: " + _oly_path.string());

			MarkClean();
		}
		else
		{
			Load(TOMLNode(), _desc.disk);

			_meta = {};
			_meta.map[detail::Key::Meta_Version] = GetVersion();
			_meta.map[detail::Key::Meta_Import] = "0";
			_meta.map[detail::Key::Meta_Type] = detail::encode_key(detail::Key::Meta_RasterFont);

			MarkDirty();
		}

		_desc.LoadFromDisk();
		
		_codepoint_counter.clear();
		for (auto& desc : _desc.scratch.glyphs)
			_codepoint_counter.increment(desc.codepoint.value);

		_glyph_model.Init(*ListAdapter());
	}

	void RasterFontDocument::DumpImpl()
	{
		toml::table table;
		Dump(table, _desc.scratch);
		_oly_path.dump_toml(table, _meta);
		_desc.WriteToDisk();
		MarkClean();
	}

	void RasterFontDocument::ResetAssetImpl()
	{
		Load(TOMLNode(), _desc.scratch);
	}

	const IDoubleDescriptor& RasterFontDocument::GetDoubleDescriptor() const
	{
		return _desc;
	}

	IDoubleDescriptor& RasterFontDocument::GetDoubleDescriptor()
	{
		return _desc;
	}

	void RasterFontDocument::Draw(RasterFontDesc& desc)
	{
		if (auto form = Form())
		{
			IMTK_DRAW_FIELDS(RASTER_FONT_PARTIAL_GENERATOR);

			if (auto subform = Subform("Glyphs"))
			{
				if (auto pause = FormPause())
				{
					_glyph_model.Update(*ListAdapter());

					if (auto scope = imtk::id_scope("##Glyph"))
					{
						_glyph_model.DrawComboHeader({ .prompt = "Select glyph", .create_tooltip = "New glyph", .delete_tooltip = "Delete glyph", .clear_tooltip = "Clear glyphs" },
							[&desc](size_t i) -> std::string {
								if (i < desc.glyphs.size() && !desc.glyphs[i].codepoint.value.empty())
									return desc.glyphs[i].codepoint.value;
								else
									return "Glyph #" + std::to_string(i);
							});
					}
				}

				if (Form::ValidActiveForm())
				{
					if (!desc.glyphs.empty())
						Draw(desc.glyphs[_glyph_model.active_index]);

					// TODO v11 preview of glyph (also in other font-related documents - e.g. preview character distance for kerning table)
				}

				if (_glyph_model.ConsumeOps(*ListAdapter()))
					MarkDirty();

				_glyph_model.active_index.ConsumeModified();
			}
		}
	}

	void RasterFontDocument::Draw(GlyphDesc& desc)
	{
		auto style_stack = GUIState::InputDataStyleSubstack();
		const bool empty_codepoint = desc.codepoint.value.empty();
		const bool duplicate_codepoint = _codepoint_counter.count(desc.codepoint.value) > 1;

		if (empty_codepoint || duplicate_codepoint)
		{
			style_stack.push(ImGuiCol_Border, Color::Error);
			style_stack.push(ImGuiStyleVar_FrameBorderSize, 1.f);
		}

		std::string previous_codepoint = desc.codepoint.value;
		desc.codepoint.draw();
		if (gui::PropertyGrid::DirtyRow())
		{
			_codepoint_counter.increment(desc.codepoint.value);
			_codepoint_counter.decrement(previous_codepoint);
		}

		style_stack.clear();

		if (gui::PropertyGrid::GetFullDrawResult().IsHovered())
		{
			if (empty_codepoint)
				ImGui::SetTooltip("Codepoint is empty");
			else if (duplicate_codepoint)
				ImGui::SetTooltip("Duplicate codepoint");
		}

		IMTK_DRAW_FIELDS(GLYPH_BODY_GENERATOR);
	}

	void RasterFontDocument::Load(TOMLNode node, RasterFontDesc& desc)
	{
		IMTK_LOAD_FIELDS(RASTER_FONT_PARTIAL_GENERATOR);

		desc.glyphs.clear();
		if (auto array = node[detail::encode_key(desc.glyphs_key)].as_array())
		{
			for (size_t i = 0; i < array->size(); ++i)
			{
				desc.glyphs.push_back();
				Load(TOMLNode(array->get(i)), desc.glyphs.back());
			}
		}
	}

	void RasterFontDocument::Load(TOMLNode node, GlyphDesc& desc)
	{
		IMTK_LOAD_FIELDS(GLYPH_GENERATOR);
	}

	void RasterFontDocument::Dump(toml::table& table, RasterFontDesc& desc)
	{
		IMTK_DUMP_FIELDS(RASTER_FONT_PARTIAL_GENERATOR);

		toml::array array;
		array.reserve(desc.glyphs.size());
		for (auto& subdesc : desc.glyphs)
			Dump(array.emplace_back<toml::table>(), subdesc);
		table.insert_or_assign(detail::encode_key(desc.glyphs_key), std::move(array));
	}

	void RasterFontDocument::Dump(toml::table& table, GlyphDesc& desc)
	{
		IMTK_DUMP_FIELDS(GLYPH_GENERATOR);
	}

	struct BriefGlyphDescPrinter
	{
		void operator()(std::ostream& os, const GlyphDesc& desc)
		{
			os << "GlyphDesc[codepoint=" << desc.codepoint.value << ", ...]";
		}
	};

	std::unique_ptr<gui::ListCallbackAdapter> RasterFontDocument::ListAdapter()
	{
		return std::make_unique<gui::ListCallbackAdapter>(gui::MakeVectorAdapter<BriefGlyphDescPrinter>(_desc.scratch.glyphs),
			gui::MakeCounterCallback(_codepoint_counter, [this](size_t i) -> const std::string& { return _desc.scratch.glyphs[i].codepoint.value; }));
	}
}
