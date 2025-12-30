#pragma once

#include "core/base/Errors.h"
#include "core/algorithms/STLUtils.h"
#include "core/util/Loader.h"

#define OLY_ENUM_EXPAND_ENUM_ENTRY(name, value) name = value,
#define OLY_ENUM_EXPAND_MAP_ENTRY(name, value) { algo::to_lower(#name), value },
#define OLY_ENUM_EXPAND_MAP_LUT_ENTRY(name, value) { value, algo::to_lower(#name) },
#define OLY_ENUM(EnumName, EntryMap)\
	struct EnumName final\
	{\
		static const std::unordered_map<std::string, unsigned int> NAMES;\
		static const std::unordered_map<unsigned int, std::string> NAMES_LUT;\
		enum E : unsigned int { EntryMap(OLY_ENUM_EXPAND_ENUM_ENTRY) } value;\
		EnumName(E value = E()) : value(value) { if (!NAMES_LUT.count((unsigned int)value)) throw Error(ErrorCode::DOES_NOT_EXIST); }\
		EnumName(unsigned int value) : value((E)value) { if (!NAMES_LUT.count(value)) throw Error(ErrorCode::DOES_NOT_EXIST); }\
		EnumName(const std::string& name)\
		{\
			auto it = NAMES.find(algo::to_lower(name));\
			if (it != NAMES.end())\
				value = (E)it->second;\
			else\
				throw Error(ErrorCode::DOES_NOT_EXIST);\
		}\
		EnumName(std::string&& name)\
		{\
			auto it = NAMES.find(algo::to_lower(std::move(name)));\
			if (it != NAMES.end())\
				value = (E)it->second;\
			else\
				throw Error(ErrorCode::DOES_NOT_EXIST);\
		}\
		operator E() const { return value; }\
		static EnumName load(TOMLNode node)\
		{\
			if (auto s = node.value<std::string>()) { try { return std::move(*s); } catch (...) {} }\
			else { unsigned int v = 0; if (io::parse_uint(node, v)) { try { return v; } catch (...) {} } }\
			return EnumName();\
		}\
		static EnumName load(TOMLNode node, EnumName def)\
		{\
			if (auto s = node.value<std::string>()) { try { return std::move(*s); } catch (...) {} }\
			else { unsigned int v = 0; if (io::parse_uint(node, v)) { try { return v; } catch (...) {} } }\
			return def;\
		}\
	}; \
	inline const std::unordered_map<std::string, unsigned int> EnumName::NAMES = { EntryMap(OLY_ENUM_EXPAND_MAP_ENTRY) };\
	inline const std::unordered_map<unsigned int, std::string> EnumName::NAMES_LUT = { EntryMap(OLY_ENUM_EXPAND_MAP_LUT_ENTRY) };
