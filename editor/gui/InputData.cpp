#include "InputData.h"

namespace oly::editor::gui
{
	imtk::item_result InputData<Color4>::operator()(const char* label, Color4& data) const
	{
		auto result = imtk::item_result::query(ImGui::ColorEdit4(label, data.ValuePtr()));
		result.modified |= imtk::prop::value::check_property(std::make_unique<imtk::prop::simple_view<Color4>>(data));
		return result;
	}
}
