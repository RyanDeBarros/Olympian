#include "FontFamilyDocument.h"

#include "core/windows/MainWindow.h"
#include "core/editor/Logger.h"

#include "definitions/Keys.h"

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

		// TODO v8
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

	void FontFamilyDocument::Load(TOMLNode node, FontFamilyDesc& desc)
	{
		// TODO v8
	}

	void FontFamilyDocument::Dump(toml::table& table, FontFamilyDesc& desc)
	{
		// TODO v8
	}
}
