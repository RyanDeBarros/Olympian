#include "FontFamilyDocument.h"

#include "core/windows/MainWindow.h"
#include "core/editor/Logger.h"

#include "gui/scopes/Form.h"
#include "gui/scopes/Subform.h"

#include "definitions/Keys.h"
#include "util/Parser.h"

namespace oly::editor
{
	const char* FontFamilyDocument::GetVersion()
	{
		return "1.0";
	}

	void FontFamilyDocument::InitImpl()
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
		auto pre_draw = PreDraw();

		gui::IDScope scope(this);
		if (auto form = Form())
		{
			DataPath path;
			Draw(path, _desc.scratch, "Regular", detail::FontStyleMode::Regular);
			Draw(path, _desc.scratch, "Bold", detail::FontStyleMode::Bold);
			Draw(path, _desc.scratch, "Italic", detail::FontStyleMode::Italic);
			Draw(path, _desc.scratch, "Bold-italic", detail::FontStyleMode::BoldItalic);
		}
	}

	void FontFamilyDocument::Load()
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
				Notification notif(LogLevel::Error, "cannot load font family - corrupted asset: " + _oly_path.string());
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
			_meta.map[detail::Key::Meta_Type] = detail::encode_key(detail::Key::Meta_FontFamily);

			MarkDirty();
		}

		_desc.LoadFromDisk();
	}

	void FontFamilyDocument::Dump()
	{
		toml::table table;
		Dump(table, _desc.scratch);
		_oly_path.dump_toml(table, _meta);
		_desc.WriteToDisk();
		MarkClean();
	}

	IDoubleDescriptor& FontFamilyDocument::GetDoubleDescriptor()
	{
		return _desc;
	}

	void FontFamilyDocument::Draw(DataPath path, FontFamilyDesc& desc, const char* subform_header, detail::FontStyleMode style)
	{
		if (auto section = Subform(subform_header))
			Draw(path / desc.subpaths.styles / desc.styles.Subpath(style), desc.styles[style]);
	}

	void FontFamilyDocument::Draw(DataPath path, FontStyleDesc& desc)
	{
		DRAW_FIELDS(STYLE_GENERATOR);
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
}
