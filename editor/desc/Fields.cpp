#include "Fields.h"

#include "definitions/Keys.h"

namespace oly::editor
{
	bool KeyIsNull(detail::Key key)
	{
		return key == detail::Key::_;
	}

	bool KeyIsNotNull(detail::Key key)
	{
		return key != detail::Key::_;
	}

	detail::Key NullKey()
	{
		return detail::Key::_;
	}

	bool BoolField::Draw()
	{
		return DescIO::Draw(label, scratch, DISK_FIELD(disk));
	}

	bool GLenumField::Draw()
	{
		return DescIO::Draw(label, scratch, DISK_FIELD(disk), names, count);
	}

	void GLenumField::Load(TOMLNode node)
	{
		scratch = Index(static_cast<GLenum>(node[detail::encode_key(key)].value_or(def)));
	}

	void GLenumField::Dump(toml::table& table) const
	{
		table.insert_or_assign(detail::encode_key(key), Scratch());
	}

	void GLenumField::Isolate()
	{
		IsolateField(*this);
	}

	void GLenumField::Reset(GLenumField& source)
	{
		ResetField(*this, source);
	}

	GLenum GLenumField::Scratch() const
	{
		return Value(scratch);
	}

	void GLenumField::SetScratch(const GLenum val)
	{
		scratch = Index(val);
	}

	GLenum GLenumField::Value(int index) const
	{
		return values[index];
	}

	int GLenumField::Index(const GLenum val) const
	{
		for (size_t i = 0; i < count; ++i)
		{
			if (val == values[i])
				return i;
		}

		return -1;
	}

	bool StringField::Draw()
	{
		return DescIO::Draw(label, scratch, DISK_FIELD(disk));
	}

	ColorField::ColorField(glm::vec4 def, detail::Key key, const char* label)
		: def(def), scratch(def), key(key), label(label)
	{
	}

	bool ColorField::Draw()
	{
		return DescIO::DrawColor(label, scratch, DISK_FIELD(disk));
	}

	void ColorField::Load(TOMLNode node)
	{
		scratch = def;
		if (KeyIsNotNull(key))
		{
			if (auto array = node[detail::encode_key(key)].as_array())
			{
				for (int i = 0; i < std::min(static_cast<int>(array->size()), 4); ++i)
				{
					if (auto v = array->get_as<double>(i))
						scratch[i] = v->get();
				}
			}
		}
	}

	void ColorField::Dump(toml::table& table) const
	{
		if (KeyIsNotNull(key))
		{
			toml::array array;
			array.reserve(4);
			array.push_back(scratch.r);
			array.push_back(scratch.g);
			array.push_back(scratch.b);
			array.push_back(scratch.a);
			table.insert_or_assign(detail::encode_key(key), std::move(array));
		}
	}

	void ColorField::Isolate()
	{
		IsolateField(*this);
	}

	void ColorField::Reset(ColorField& source)
	{
		ResetField(*this, source);
	}

	void LoadStringArray(TOMLNode node, std::string* strings, size_t count)
	{
		if (auto array = node.as_array())
		{
			for (size_t i = 0; i < std::min(array->size(), count); ++i)
			{
				if (auto s = array->get_as<std::string>(i))
					strings[i] = s->get();
			}
		}
	}

	toml::array DumpStringArray(const std::string* strings, size_t count)
	{
		toml::array array;
		array.reserve(count);
		for (size_t i = 0; i < count; ++i)
			array.push_back(strings[i]);
		return array;
	}
}
