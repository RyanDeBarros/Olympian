#include "RasterFontDocument.h"

#include "core/windows/MainWindow.h"
#include "core/editor/Logger.h"
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
		{
			Notification notif(LogLevel::Warning, "Asset is not located in resource folder");
			MainWindow::Instance().PushNotification(std::move(notif));
		}

		LoadAsset();
	}

	void RasterFontDocument::Draw()
	{
		auto pre_draw = PreDraw();

		gui::IDScope scope(this);
		Draw(DataPath(), _desc.scratch);
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
			{
				Notification notif(LogLevel::Error, "cannot load raster font - corrupted asset: " + _oly_path.string());
				MainWindow::Instance().PushNotification(std::move(notif));
			}

			MarkClean();
		}
		else
		{
			Load(TOMLNode(), _desc.disk);

			_meta = {};
			_meta.map[detail::Key::Meta_Version] = "1.0";
			_meta.map[detail::Key::Meta_Import] = "1";
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

	const IDoubleDescriptor& RasterFontDocument::GetDoubleDescriptor() const
	{
		return _desc;
	}

	IDoubleDescriptor& RasterFontDocument::GetDoubleDescriptor()
	{
		return _desc;
	}

	void RasterFontDocument::Draw(DataPath path, RasterFontDesc& desc)
	{
		if (auto form = Form())
		{
			DRAW_FIELDS(RASTER_FONT_PARTIAL_GENERATOR);

			if (auto subform = Subform("Glyphs"))
			{
				if (auto pause = FormPause())
				{
					_glyph_model.Update(*ListAdapter());

					if (auto scope = gui::IDScope("##Glyph"))
					{
						_glyph_model.DrawComboHeader({ .prompt = "Select glyph", .create_tooltip = "New glyph", .delete_tooltip = "Delete glyph", .clear_tooltip = "Clear glyphs" },
							[&desc](size_t i) -> std::string {
								if (i < desc.glyphs.Size() && !desc.glyphs[i].codepoint.value.empty())
									return desc.glyphs[i].codepoint.value;
								else
									return "Glyph #" + std::to_string(i);
							});
					}
				}

				if (Form::ValidActiveForm())
				{
					if (!desc.glyphs.Empty())
						Draw(path / desc.subpaths.glyphs / desc.glyphs.Subpath(_glyph_model.active_index), desc.glyphs[_glyph_model.active_index]);

					// TODO v11 preview of glyph (also in other font-related documents - e.g. preview character distance for kerning table)
				}

				if (_glyph_model.ConsumeOps(*ListAdapter()))
					MarkDirty();

				_glyph_model.active_index.ConsumeModified();
			}
		}
	}

	void RasterFontDocument::Draw(DataPath path, GlyphDesc& desc)
	{
		GUIState::InputDataStyleStack style_stack;
		const bool empty_codepoint = desc.codepoint.value.empty();
		const bool duplicate_codepoint = _codepoint_counter.count(desc.codepoint.value) > 1;

		if (empty_codepoint || duplicate_codepoint)
		{
			style_stack.PushStyle(gui::StyleColorCtor{ .idx = ImGuiCol_Border, .col = Color::Error });
			style_stack.PushStyle(gui::StyleVar1DCtor{ .idx = ImGuiStyleVar_FrameBorderSize, .value = 1.f });
		}

		std::string previous_codepoint = desc.codepoint.value;
		DRAW_FIELD(codepoint);
		desc.codepoint.Draw(path / desc.subpaths.codepoint);
		if (gui::PropertyGrid::DirtyRow())
		{
			_codepoint_counter.increment(desc.codepoint.value);
			_codepoint_counter.decrement(previous_codepoint);
		}

		style_stack.PopStyles();

		if (gui::PropertyGrid::GetFullDrawResult().IsHovered())
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

	struct BriefGlyphDescPrinter
	{
		void operator()(std::ostream& os, const GlyphDesc& desc)
		{
			os << "GlyphDesc[codepoint=" << desc.codepoint.value << ", ...]";
		}
	};

	std::unique_ptr<gui::ListCallbackAdapter> RasterFontDocument::ListAdapter()
	{
		return std::make_unique<gui::ListCallbackAdapter>(_desc.scratch.glyphs.ListAdapter<BriefGlyphDescPrinter>(DataPath() / _desc.scratch.subpaths.glyphs),
			gui::MakeCounterCallback(_codepoint_counter, [this](size_t i) -> const std::string& { return _desc.scratch.glyphs[i].codepoint.value; }));
	}
}
