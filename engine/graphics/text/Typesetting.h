#pragma once

#include "graphics/text/ParagraphFormat.h"
#include "external/GL.h"

namespace oly::rendering::internal
{
	struct PageLayout
	{
		glm::vec2 content_size;
		glm::vec2 fitted_size;
	};

	struct PageBuildData
	{
		struct Line
		{
			float width = 0.0f;
			float max_height = 0.0f;
			float space_width = 0.0f;
			float final_advance_width = 0.0f;
			GLuint characters = 0;

			float spaced_height(const ParagraphFormat& format) const { return max_height * (width == 0.0f ? format.linebreak_spacing : format.line_spacing); }
		};

		std::vector<Line> lines;
		GLuint linebreaks = 0;

		const Line& current_line() const { return lines.back(); }
		Line& current_line() { return lines.back(); }
	};

	struct TypesetData
	{
		float x = 0.0f, y = 0.0f;
		GLuint character = 0;
		size_t line = 0;
	};

	struct AlignmentCache
	{
		struct Line
		{
			float start = 0.0f;
			float character_shift = 0.0f;
			float space_width_mult = 1.0f;
			float height = 0.0f;
		};

		std::vector<Line> lines;

		glm::vec2 pivot_offset = {};
		glm::vec2 padding_offset = {};
		float valign_offset = 0.0f;

		glm::vec2 position(const TypesetData& typeset) const
		{
			return pivot_offset + padding_offset + glm::vec2{
				typeset.x + lines[typeset.line].start + lines[typeset.line].character_shift * typeset.character,
				typeset.y + valign_offset
			};
		}
	};
}
