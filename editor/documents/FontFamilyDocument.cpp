#include "FontFamilyDocument.h"

#include "core/editor/Notifier.h"

#include "assets/TranslateKey.h"
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
			Notifier::NotifyWarning("Asset is not located in resource folder");

		LoadAsset();
	}

	void FontFamilyDocument::Draw()
	{
		auto pre_draw = PreDraw();

		imtk::id_scope scope(this);
		if (auto form = imtk::prop::form())
		{
			Draw(_desc.scratch, "Regular", detail::FontStyleMode::Regular);
			Draw(_desc.scratch, "Bold", detail::FontStyleMode::Bold);
			Draw(_desc.scratch, "Italic", detail::FontStyleMode::Italic);
			Draw(_desc.scratch, "Bold-italic", detail::FontStyleMode::BoldItalic);
		}
	}

	void FontFamilyDocument::LoadImpl()
	{
		if (_oly_path.is_file())
		{
			_meta = detail::MetaSplitter::decode_meta(_oly_path);

			toml::table table;
			std::string err = _oly_path.load_toml(table);
			if (err.empty())
				Load(imtk::toml_node(table), _desc.disk);
			else
				Notifier::NotifyError("cannot load font family - corrupted asset: " + _oly_path.string());

			MarkClean();
		}
		else
		{
			Load(imtk::toml_node(), _desc.disk);

			_meta = {};
			_meta.map[detail::Key::Meta_Version] = GetVersion();
			_meta.map[detail::Key::Meta_Import] = "0";
			_meta.map[detail::Key::Meta_Type] = detail::encode_key(detail::Key::Meta_FontFamily);

			MarkDirty();
		}

		_desc.LoadFromDisk();
	}

	void FontFamilyDocument::DumpImpl()
	{
		toml::table table;
		Dump(table, _desc.scratch);
		_oly_path.dump_toml(table, _meta);
		_desc.WriteToDisk();
		MarkClean();
	}

	void FontFamilyDocument::ResetAssetImpl()
	{
		Load(imtk::toml_node(), _desc.scratch);
	}

	const IDoubleDescriptor& FontFamilyDocument::GetDoubleDescriptor() const
	{
		return _desc;
	}

	IDoubleDescriptor& FontFamilyDocument::GetDoubleDescriptor()
	{
		return _desc;
	}

	void FontFamilyDocument::Draw(FontFamilyDesc& desc, const char* subform_header, detail::FontStyleMode style)
	{
		if (auto subform = imtk::prop::subform(subform_header))
			Draw(desc.styles[style]);
	}

	void FontFamilyDocument::Draw(FontStyleDesc& desc)
	{
		IMTK_DRAW_FIELDS(STYLE_GENERATOR);
	}

	void FontFamilyDocument::Load(imtk::toml_node node, FontFamilyDesc& desc)
	{
		desc.styles.clear();

		if (auto table = desc.styles.subnode(node).as_table())
		{
			for (auto&& [key, subnode] : *table)
			{
				if (auto style = stoi(key.str()))
					Load(imtk::toml_node(subnode), desc.styles[static_cast<detail::FontStyleMode>(*style)]);
			}
		}
	}

	void FontFamilyDocument::Load(imtk::toml_node node, FontStyleDesc& desc)
	{
		IMTK_LOAD_FIELDS(STYLE_GENERATOR);
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
		desc.styles.dump_into(table, std::move(subtable));
	}

	void FontFamilyDocument::Dump(toml::table& table, FontStyleDesc& desc)
	{
		IMTK_DUMP_FIELDS(STYLE_GENERATOR);
	}
}
