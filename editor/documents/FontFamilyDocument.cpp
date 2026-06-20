#include "FontFamilyDocument.h"

#include "core/windows/MainWindow.h"
#include "core/editor/Logger.h"

#include "gui/scopes/CollapsingSection.h"

#include "definitions/Keys.h"
#include "util/Parser.h"

namespace oly::editor
{
	const char* FontFamilyDocument::GetVersion()
	{
		return "1.0";
	}

	void FontFamilyDocument::Init()
	{
		if (!GetOlyPath().is_resource())
		{
			Notification notif(LogLevel::Warning, "Asset is not located in resource folder");
			MainWindow::Instance().PushNotification(std::move(notif));
		}

		Load();
	}

	void FontFamilyDocument::Draw()
	{
		gui::IDScope scope(this);
		Draw(_scratch);
	}

	void FontFamilyDocument::Load()
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
				Notification notif(LogLevel::Error, "cannot load font family - corrupted asset: " + _oly_path.string());
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
			_meta.map[detail::Key::Meta_Type] = detail::encode_key(detail::Key::Meta_FontFamily);

			MarkDirty();
		}

		_scratch = _disk;
	}

	void FontFamilyDocument::Dump()
	{
		toml::table table;
		Dump(table, _scratch);
		_oly_path.dump_toml(table, _meta);
		_disk = _scratch;
		MarkClean();
	}

	void FontFamilyDocument::Draw(FontFamilyDesc& desc)
	{
		if (auto section = CollapsingSection("Regular"))
			Draw(GetFontStyleDesc(detail::FontStyleMode::Regular));

		if (auto section = CollapsingSection("Bold"))
			Draw(GetFontStyleDesc(detail::FontStyleMode::Bold));

		if (auto section = CollapsingSection("Italic"))
			Draw(GetFontStyleDesc(detail::FontStyleMode::Italic));

		if (auto section = CollapsingSection("Bold-italic"))
			Draw(GetFontStyleDesc(detail::FontStyleMode::BoldItalic));
	}
	
	void FontFamilyDocument::Draw(FontStyleDesc& desc)
	{
		if (auto form = Form())
		{
			DRAW_FIELDS(STYLE_GENERATOR);
		}
	}

	void FontFamilyDocument::Load(TOMLNode node, FontFamilyDesc& desc)
	{
		desc.styles.Clear();

		if (auto table = node[detail::encode_key(desc.styles_key)].as_table())
		{
			for (auto&& [key, subnode] : *table)
			{
				auto style = stoi(key.str());
				if (!style)
					continue;

				FontStyleDesc subdesc;
				Load(TOMLNode(subnode), subdesc);
				desc.styles.map.emplace(static_cast<detail::FontStyleMode>(*style), std::move(subdesc));
			}
		}
	}

	void FontFamilyDocument::Load(TOMLNode node, FontStyleDesc& desc)
	{
		LOAD_FIELDS(STYLE_GENERATOR);
	}

	void FontFamilyDocument::Dump(toml::table& table, FontFamilyDesc& desc)
	{
		toml::table subtable;
		for (auto&& [style, subdesc] : desc.styles)
		{
			toml::table inner;
			Dump(inner, subdesc);
			subtable.insert_or_assign(std::to_string(style), std::move(inner));
		}
		table.insert_or_assign(detail::encode_key(desc.styles_key), std::move(subtable));
	}

	void FontFamilyDocument::Dump(toml::table& table, FontStyleDesc& desc)
	{
		DUMP_FIELDS(STYLE_GENERATOR);
	}

	FontStyleDesc& FontFamilyDocument::GetFontStyleDesc(detail::FontStyleMode style)
	{
		return _scratch.styles[style];
	}
}
