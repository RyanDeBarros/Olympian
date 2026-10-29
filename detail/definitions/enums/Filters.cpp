#include "Filters.h"

namespace oly::detail
{
	const GLenum kMinFilterValues[kMinFilterCount] = {
		GL_NEAREST,
		GL_LINEAR,
		GL_NEAREST_MIPMAP_NEAREST,
		GL_LINEAR_MIPMAP_NEAREST,
		GL_NEAREST_MIPMAP_LINEAR,
		GL_LINEAR_MIPMAP_LINEAR
	};

	const char* const kMinFilterNames[kMinFilterCount] = {
		"Nearest",
		"Linear",
		"Nearest (Nearest Mipmap)",
		"Linear (Nearest Mipmap)",
		"Nearest (Linear Mipmap)",
		"Linear (Linear Mipmap)"
	};

	const GLenum kMagFilterValues[kMagFilterCount] = {
		GL_NEAREST,
		GL_LINEAR,
	};

	const char* const kMagFilterNames[kMagFilterCount] = {
		"Nearest",
		"Linear",
	};

	const GLenum kWrapValues[kWrapCount] = {
		GL_CLAMP_TO_EDGE,
		GL_CLAMP_TO_BORDER,
		GL_MIRRORED_REPEAT,
		GL_REPEAT,
		GL_MIRROR_CLAMP_TO_EDGE
	};

	const char* const kWrapNames[kWrapCount] = {
		"Clamp To Edge",
		"Clamp To Border",
		"Repeat (Mirrored)",
		"Repeat",
		"Clamp To Edge (Mirrored)"
	};
}
