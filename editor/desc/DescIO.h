#pragma once

#include "assets/TranslateKey.h"
#include "definitions/Enums.h"

#include "GL.h"
#include "TOML.h"

#include <optional>

namespace oly::detail
{
	enum class Key : unsigned long long;
}

namespace oly::editor
{
	struct DescIO
	{
		static bool BeginForm(void* id);
		static void EndForm();

		static bool Draw(const char* label, bool& data, const bool* disk);
		static bool Draw(const char* label, int& data, const int* disk, std::optional<int> min, std::optional<int> max);
		static bool Draw(const char* label, float& data, const float* disk, std::optional<float> min, std::optional<float> max);
		static bool Draw(const char* label, GLenum& data, const GLenum* disk, const GLenum* values, const char** names, size_t count);
		static bool Draw(const char* label, detail::StorageMode& data, const detail::StorageMode* disk);
		static bool Draw(const char* label, detail::SVGMipmapGenerationMode& data, const detail::SVGMipmapGenerationMode* disk);

		static void Load(TOMLNode node, bool& data, detail::Key key, bool def);
		static void Load(TOMLNode node, int& data, detail::Key key, int def);
		static void Load(TOMLNode node, float& data, detail::Key key, float def);
		static void Load(TOMLNode node, GLenum& data, detail::Key key, GLenum def);

		template<typename T> requires (std::is_enum_v<T>)
		static void Load(TOMLNode node, T& data, detail::Key key, T def)
		{
			data = static_cast<T>(node[detail::encode_key(key)].value_or(static_cast<int64_t>(def)));
		}

		static void Dump(toml::table& table, detail::Key key, bool data);
		static void Dump(toml::table& table, detail::Key key, int data);
		static void Dump(toml::table& table, detail::Key key, float data);
		static void Dump(toml::table& table, detail::Key key, GLenum data);

		template<typename T> requires (std::is_enum_v<T>)
		static void Dump(toml::table& table, detail::Key key, T data)
		{
			table.insert_or_assign(detail::encode_key(key), static_cast<int64_t>(data));
		}
	};

#define OLY_EDITOR_DESC_IO_REVERT_PAIR(desc, disk, member) desc.member, disk ? &disk->member : nullptr
#define OLY_EDITOR_DESC_IO_DRAW_FIELD(label, field, ...) if (DescIO::Draw(label, OLY_EDITOR_DESC_IO_REVERT_PAIR(desc, disk, field), __VA_ARGS__)) MarkDirty();
}
