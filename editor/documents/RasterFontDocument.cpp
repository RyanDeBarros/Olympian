#include "RasterFontDocument.h"

#include "core/windows/MainWindow.h"
#include "core/editor/Logger.h"

#include "gui/Subform.h"

#include "definitions/Keys.h"

namespace oly::editor
{
	const char* RasterFontDocument::GetVersion()
	{
		return "1.0";
	}

	void RasterFontDocument::Init()
	{
		if (!GetOlyPath().is_resource())
		{
			Notification notif(LogLevel::Warning, "Asset is not located in resource folder");
			MainWindow::Instance().PushNotification(std::move(notif));
		}

		Load();
	}

	void RasterFontDocument::Draw()
	{
		gui::IDScope scope(this);
		Draw(_scratch);
	}

	void RasterFontDocument::Load()
	{
		if (_oly_path.is_file())
		{
			_meta = detail::MetaSplitter::decode_meta(_oly_path);

			toml::table table;
			std::string err = _oly_path.load_toml(table);
			if (err.empty())
				Load(TOMLNode(table), _disk);
			else
			{
				Notification notif(LogLevel::Error, "cannot load raster font - corrupted asset: " + _oly_path.string());
				MainWindow::Instance().PushNotification(std::move(notif));
			}

			MarkClean();
		}
		else
		{
			Load(TOMLNode(), _disk);

			_meta = {};
			_meta.map[detail::Key::Meta_Version] = "1.0";
			_meta.map[detail::Key::Meta_Import] = "1";
			_meta.map[detail::Key::Meta_Type] = detail::encode_key(detail::Key::Meta_RasterFont);

			MarkDirty();
		}

		_scratch = _disk;
		
		_codepoint_counter.clear();
		for (auto& desc : _scratch.glyphs)
			_codepoint_counter.increment(desc.codepoint.scratch);

		_glyph_model.Init(*ListAdapter());
	}

	void RasterFontDocument::Dump()
	{
		toml::table table;
		Dump(table, _scratch);
		_oly_path.dump_toml(table, _meta);
		_disk = _scratch;
		MarkClean();
	}

	void RasterFontDocument::Draw(RasterFontDesc& desc)
	{
		if (auto form = Form())
		{
			DRAW_FIELDS(RASTER_FONT_PARTIAL_GENERATOR);

			if (auto subform = Subform(form, "Glyphs"))
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Text("Select Glyph");

				ImGui::TableNextColumn();
				_glyph_model.DrawComboHeader([&desc](size_t i) -> std::string {
					if (i < desc.glyphs.Size() && !desc.glyphs[i].codepoint.scratch.empty())
						return desc.glyphs[i].codepoint.scratch;
					else
						return "Glyph #" + std::to_string(i);
				}, "New glyph", "Delete glyph", "Clear glyphs");

				if (!desc.glyphs.Empty())
					Draw(_scratch.glyphs[_glyph_model.active_index]);

				if (_glyph_model.ConsumeOps(*ListAdapter()))
					MarkDirty();

				_glyph_model.active_index.ConsumeModified();
				// TODO v9 preview of glyph (also in other font-related documents - e.g. preview character distance for kerning table)
			}
		}
	}

	void RasterFontDocument::Draw(GlyphDesc& desc)
	{
		const bool empty_codepoint = desc.codepoint.scratch.empty();
		const bool duplicate_codepoint = _codepoint_counter.count(desc.codepoint.scratch) > 1;

		if (empty_codepoint || duplicate_codepoint)
		{
			ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(255, 0, 0, 255));
			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
		}

		std::string codepoint = desc.codepoint.scratch;
		// TODO v8 style/var gets passed to reset button as well -> pass DrawSettings to Draw() so push/pop only happens around the InputData widget
		auto codepoint_result = desc.codepoint.Draw();
		if (codepoint_result)
		{
			_codepoint_counter.increment(desc.codepoint.scratch);
			_codepoint_counter.decrement(codepoint);
			MarkDirty();
		}

		if (empty_codepoint || duplicate_codepoint)
		{
			ImGui::PopStyleVar();
			ImGui::PopStyleColor();
		}

		if (codepoint_result.IsHovered())
		{
			if (empty_codepoint)
				ImGui::SetTooltip("Codepoint is empty");
			else if (duplicate_codepoint)
				ImGui::SetTooltip("Duplicate codepoint");
		}

		DRAW_FIELDS(GLYPH_BODY_GENERATOR);
	}

	void RasterFontDocument::Load(TOMLNode node, RasterFontDesc& desc)
	{
		LOAD_FIELDS(RASTER_FONT_PARTIAL_GENERATOR);

		desc.glyphs.Clear();
		if (auto array = node[detail::encode_key(desc.glyphs_key)].as_array())
		{
			for (size_t i = 0; i < array->size(); ++i)
			{
				desc.glyphs.PushBack();
				Load(TOMLNode(array->get(i)), desc.glyphs.Back());
			}
		}
	}

	void RasterFontDocument::Load(TOMLNode node, GlyphDesc& desc)
	{
		LOAD_FIELDS(GLYPH_GENERATOR);
	}

	void RasterFontDocument::Dump(toml::table& table, RasterFontDesc& desc)
	{
		DUMP_FIELDS(RASTER_FONT_PARTIAL_GENERATOR);

		toml::array array;
		array.reserve(desc.glyphs.Size());
		for (auto& subdesc : desc.glyphs)
		{
			toml::table subtable;
			Dump(subtable, subdesc);
			array.push_back(std::move(subtable));
		}
		table.insert_or_assign(detail::encode_key(desc.glyphs_key), std::move(array));
	}

	void RasterFontDocument::Dump(toml::table& table, GlyphDesc& desc)
	{
		DUMP_FIELDS(GLYPH_GENERATOR);
	}

	std::unique_ptr<gui::ListCallbackAdapter> RasterFontDocument::ListAdapter()
	{
		return std::make_unique<gui::ListCallbackAdapter>(_scratch.glyphs.ListAdapter(),
			gui::MakeCounterCallback(_codepoint_counter, [this](size_t i) -> const std::string& { return _scratch.glyphs[i].codepoint.scratch; }));
	}
}
