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
		// TODO v8 set glyph model policy to 'RequireUnique'?
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

		_glyph_model.Init(*_scratch.glyphs.ListAdapter());
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

				if (_glyph_model.ConsumeOps(*_scratch.glyphs.ListAdapter()))
					MarkDirty();

				_glyph_model.active_index.ConsumeModified();
				// TODO v8 preview of glyph (also in other font-related documents)? -> maybe in future branch
			}
		}
	}

	void RasterFontDocument::Draw(GlyphDesc& desc)
	{
		DRAW_FIELDS(GLYPH_GENERATOR);
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
}
