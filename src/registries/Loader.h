#pragma once

#include "external/GL.h"
#include "external/TOML.h"
#include "core/base/Transforms.h"

namespace oly::reg
{
	extern toml::v3::parse_result load_toml(const char* file);
	extern toml::v3::parse_result load_toml(const std::string& file);

	extern bool parse_int(const TOMLNode& node, const std::string& name, int& v);
	extern bool parse_int(const CTOMLNode& node, const std::string& name, int& v);
	extern bool parse_int(const toml::table& node, const std::string& name, int& v);
	extern bool parse_float(const TOMLNode& node, const std::string& name, float& v);
	extern bool parse_float(const CTOMLNode& node, const std::string& name, float& v);
	extern bool parse_float(const toml::table& node, const std::string& name, float& v);
	extern bool parse_vec2(const toml::v3::array* arr, glm::vec2& v);
	inline bool parse_vec2(const TOMLNode& node, const std::string& name, glm::vec2& v) { return parse_vec2(node[name].as_array(), v); }
	inline bool parse_vec2(const CTOMLNode& node, const std::string& name, glm::vec2& v) { return parse_vec2(node[name].as_array(), v); }
	inline bool parse_vec2(const toml::table& node, const std::string& name, glm::vec2& v) { return parse_vec2(node[name].as_array(), v); }
	extern bool parse_ivec2(const toml::v3::array* arr, glm::ivec2& v);
	inline bool parse_ivec2(const TOMLNode& node, const std::string& name, glm::ivec2& v) { return parse_ivec2(node[name].as_array(), v); }
	inline bool parse_ivec2(const CTOMLNode& node, const std::string& name, glm::ivec2& v) { return parse_ivec2(node[name].as_array(), v); }
	inline bool parse_ivec2(const toml::table& node, const std::string& name, glm::ivec2& v) { return parse_ivec2(node[name].as_array(), v); }
	extern bool parse_vec4(const toml::v3::array* arr, glm::vec4& v);
	inline bool parse_vec4(const TOMLNode& node, const std::string& name, glm::vec4& v) { return parse_vec4(node[name].as_array(), v); }
	inline bool parse_vec4(const CTOMLNode& node, const std::string& name, glm::vec4& v) { return parse_vec4(node[name].as_array(), v); }
	inline bool parse_vec4(const toml::table& node, const std::string& name, glm::vec4& v) { return parse_vec4(node[name].as_array(), v); }
	extern Transform2D load_transform_2d(const TOMLNode& node);
	extern Transform2D load_transform_2d(const CTOMLNode& node);
	inline Transform2D load_transform_2d(const toml::table& node, const char* name) { return load_transform_2d((const TOMLNode&)node[name]); }
	inline Transform2D load_transform_2d(const TOMLNode& node, const char* name) { return load_transform_2d(node[name]); }

	extern bool parse_mag_filter(const TOMLNode& node, const std::string& name, GLenum& mag_filter);
	extern bool parse_mag_filter(const toml::table& node, const std::string& name, GLenum& mag_filter);
	extern bool parse_min_filter(const TOMLNode& node, const std::string& name, GLenum& min_filter);
	extern bool parse_min_filter(const toml::table& node, const std::string& name, GLenum& min_filter);
	extern bool parse_wrap(const TOMLNode& node, const std::string& name, GLenum& wrap);
	extern bool parse_wrap(const toml::table& node, const std::string& name, GLenum& wrap);
}
