#pragma once

#define OLY_DETAIL_IMPLEMENT_BITMASK(Enum) \
inline Enum operator|(Enum lhs, Enum rhs) { return static_cast<Enum>(static_cast<int>(lhs) | static_cast<int>(rhs)); } \
inline Enum operator&(Enum lhs, Enum rhs) { return static_cast<Enum>(static_cast<int>(lhs) & static_cast<int>(rhs)); } \
inline Enum operator^(Enum lhs, Enum rhs) { return static_cast<Enum>(static_cast<int>(lhs) ^ static_cast<int>(rhs)); } \
inline Enum operator~(Enum value) {	return static_cast<Enum>(~static_cast<int>(value)); } \
inline Enum& operator|=(Enum& lhs, Enum rhs) { lhs = lhs | rhs; return lhs; } \
inline Enum& operator&=(Enum& lhs, Enum rhs) { lhs = lhs & rhs; return lhs; } \
inline Enum& operator^=(Enum& lhs, Enum rhs) { lhs = lhs ^ rhs; return lhs; }

#define OLY_DETAIL_DECLARE_NESTED_BITMASK(Enum) \
friend Enum operator|(Enum, Enum); \
friend Enum operator&(Enum, Enum); \
friend Enum operator^(Enum, Enum); \
friend Enum operator~(Enum); \
friend Enum& operator|=(Enum&, Enum); \
friend Enum& operator&=(Enum&, Enum); \
friend Enum& operator^=(Enum&, Enum);
