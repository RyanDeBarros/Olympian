#include "Typesetting.h"

namespace oly::rendering::internal
{
    float PageBuildData::Line::spaced_height(const ParagraphFormat& format) const
    {
        return max_height * (width == 0.0f ? format.linebreak_spacing : format.line_spacing);
    }

    const PageBuildData::Line& PageBuildData::current_line() const
    {
        return lines.back();
    }

    PageBuildData::Line& PageBuildData::current_line()
    {
        return lines.back();
    }

    glm::vec2 AlignmentCache::position(const TypesetData& typeset) const
    {
        return pivot_offset + padding_offset + glm::vec2{
            typeset.x + lines[typeset.line].start + lines[typeset.line].character_shift * typeset.character,
            typeset.y + valign_offset
        };
    }
}
