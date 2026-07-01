#include "CommonBufferPreset.h"

namespace oly::detail
{
	const char* buffer_of(CommonBufferPreset preset)
	{
		switch (preset)
		{
		case CommonBufferPreset::Common:
			return "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,./<>?;:\'\"\\|[]{}!@#$%^&*()-=_+`~";
		case CommonBufferPreset::AlphaNumeric:
			return "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
		case CommonBufferPreset::Numeric:
			return "0123456789";
		case CommonBufferPreset::Alphabet:
			return "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
		case CommonBufferPreset::AlphabetLowercase:
			return "abcdefghijklmnopqrstuvwxyz";
		case CommonBufferPreset::AlphabetUppercase:
			return "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
		default:
			return "";
		}
	}

	std::ostream& operator<<(std::ostream& os, CommonBufferPreset preset)
	{
		os << "CommonBufferPreset(";

		switch (preset)
		{
		case CommonBufferPreset::Common:
			os << "Common";
			break;

		case CommonBufferPreset::AlphaNumeric:
			os << "AlphaNumeric";
			break;

		case CommonBufferPreset::Numeric:
			os << "Numeric";
			break;

		case CommonBufferPreset::Alphabet:
			os << "Alphabet";
			break;

		case CommonBufferPreset::AlphabetLowercase:
			os << "AlphabetLowercase";
			break;

		case CommonBufferPreset::AlphabetUppercase:
			os << "AlphabetUppercase";
			break;

		default:
			os << "unknown";
			break;
		}

		return os << ")";
	}
}
