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
}
