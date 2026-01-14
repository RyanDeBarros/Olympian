#pragma once

#include "core/base/Errors.h"
#include "core/util/StringParam.h"
#include "core/util/Loader.h"

#define OLY_ENUM_EXPAND_ENUM_ENTRY(name, value) name = value,
#define OLY_ENUM_EXPAND_MAP_ENTRY(name, value) { oly::StringParam(std::string(#name)).to_lower().transfer(), value },
#define OLY_ENUM_EXPAND_MAP_LUT_ENTRY(name, value) { value, oly::StringParam(std::string(#name)).to_lower().transfer() },
#define OLY_ENUM(EnumName, EntryMap)\
	struct EnumName final\
	{\
		static const std::unordered_map<std::string, unsigned int, oly::StringParamHeteroHash, oly::StringParamHeteroEqual> NAMES;\
		static const std::unordered_map<unsigned int, std::string> NAMES_LUT;\
		enum E : unsigned int { EntryMap(OLY_ENUM_EXPAND_ENUM_ENTRY) } value;\
		EnumName(E value = E()) : value(value) { if (!NAMES_LUT.count((unsigned int)value)) throw Error(ErrorCode::DOES_NOT_EXIST); }\
		EnumName(unsigned int value) : value((E)value) { if (!NAMES_LUT.count(value)) throw Error(ErrorCode::DOES_NOT_EXIST); }\
		EnumName(const oly::StringParam& name)\
		{\
			auto it = NAMES.find(oly::StringParam(name).to_lower());\
			if (it != NAMES.end())\
				value = (E)it->second;\
			else\
				throw Error(ErrorCode::DOES_NOT_EXIST);\
		}\
		operator E() const { return value; }\
		static EnumName load(TOMLNode node)\
		{\
			if (auto s = node.value<std::string>()) { try { return oly::StringParam(std::move(*s)); } catch (...) {} }\
			else { unsigned int v = 0; if (io::parse_uint(node, v)) { try { return v; } catch (...) {} } }\
			throw Error(ErrorCode::LOAD_ENUM);\
		}\
		static EnumName load(TOMLNode node, EnumName def)\
		{\
			if (auto s = node.value<std::string>()) { try { return oly::StringParam(std::move(*s)); } catch (...) {} }\
			else { unsigned int v = 0; if (io::parse_uint(node, v)) { try { return v; } catch (...) {} } }\
			return def;\
		}\
	}; \
	inline const std::unordered_map<std::string, unsigned int, oly::StringParamHeteroHash, oly::StringParamHeteroEqual> EnumName::NAMES = { EntryMap(OLY_ENUM_EXPAND_MAP_ENTRY) };\
	inline const std::unordered_map<unsigned int, std::string> EnumName::NAMES_LUT = { EntryMap(OLY_ENUM_EXPAND_MAP_LUT_ENTRY) };\
