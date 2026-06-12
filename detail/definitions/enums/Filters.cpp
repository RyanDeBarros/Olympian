#include "Filters.h"

namespace oly::detail
{
	const GLenum MIN_FILTER_VALUES[MIN_FILTER_COUNT] = {
		GL_NEAREST,
		GL_LINEAR,
		GL_NEAREST_MIPMAP_NEAREST,
		GL_LINEAR_MIPMAP_NEAREST,
		GL_NEAREST_MIPMAP_LINEAR,
		GL_LINEAR_MIPMAP_LINEAR
	};

	const char* MIN_FILTER_NAMES[MIN_FILTER_COUNT] = {
		"Nearest",
		"Linear",
		"Nearest (Nearest Mipmap)",
		"Linear (Nearest Mipmap)",
		"Nearest (Linear Mipmap)",
		"Linear (Linear Mipmap)"
	};

	const GLenum MAG_FILTER_VALUES[MAG_FILTER_COUNT] = {
		GL_NEAREST,
		GL_LINEAR,
	};

	const char* MAG_FILTER_NAMES[MAG_FILTER_COUNT] = {
		"Nearest",
		"Linear",
	};

	const GLenum WRAP_VALUES[WRAP_COUNT] = {
		GL_CLAMP_TO_EDGE,
		GL_CLAMP_TO_BORDER,
		GL_MIRRORED_REPEAT,
		GL_REPEAT,
		GL_MIRROR_CLAMP_TO_EDGE
	};

	const char* WRAP_NAMES[WRAP_COUNT] = {
		"Clamp To Edge",
		"Clamp To Border",
		"Repeat (Mirrored)",
		"Repeat",
		"Clamp To Edge (Mirrored)"
	};
}
