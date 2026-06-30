#include "DescIO.h"

#include "gui/InlineWidget.h"
#include "gui/scopes/DisabledSection.h"
#include "gui/scopes/Subform.h"

#include "core/MemoryUnit.h"

#include "definitions/Keys.h"
#include "definitions/enums/Include.h"

#include <imgui.h>

#include <span>

// TODO DEBT support more complex property views. For example, a dynamic list of strings should be able to paste into another, even though they might have different sizes. Another example is dynamic descriptors, such as checkoboxes or combos enabling/disabling sections.

namespace oly::editor
{
	void DescIO::Draw(const char* label, int& data, const int& def, LabelSpanRegistry::Handle names)
	{
		RowInputData(label, data, def, names);
	}

	void DescIO::Draw(const char* label, EditSession<std::string>* data, const std::string* def, size_t count)
	{
		const auto generator = [data, count](PropertyPage& props) {
			PropertyRow row;
			for (size_t i = 0; i < count; ++i)
				row.list.push_back(std::make_unique<prop::PrimitivePropertyView<std::string>>(data[i].buffer));

			props.page.push_back(std::move(row));
			};

		if (auto subform = Subform(label, generator))
		{
			for (size_t i = 0; i < count; ++i)
				RowInputData(std::to_string(i).c_str(), data[i], def[i]);
		}
	}

	void DescIO::Draw(const char* label, EditSession<std::string>* data, const std::string* def, const char** sublabels, size_t count)
	{
		const auto generator = [data, count](PropertyPage& props) {
			PropertyRow row;
			for (size_t i = 0; i < count; ++i)
				row.list.push_back(std::make_unique<prop::PrimitivePropertyView<std::string>>(data[i].buffer));

			props.page.push_back(std::move(row));
			};

		if (auto subform = Subform(label, generator))
		{
			for (size_t i = 0; i < count; ++i)
				RowInputData(sublabels[i], data[i], def[i]);
		}
	}

	void DescIO::Draw(const char* label, bool* data, const bool* def, const char** sublabels, size_t count, bool inline_checkboxes)
	{
		Draw(label, data, def, sublabels, nullptr, count, inline_checkboxes);
	}

	void DescIO::Draw(const char* label, bool* data, const bool* def, const char** sublabels, const bool* disabled, size_t count, bool inline_checkboxes)
	{
		gui::IDScope scope(data);
		gui::PropertyGrid::Key::SetLabel(label);

		for (size_t i = 0; i < count; ++i)
		{
			if (data[i] != def[i])
			{
				gui::PropertyGrid::Reset::Button();
				break;
			}
		}

		gui::PropertyGrid::Value::AddComponent(comp::Generic([&data, sublabels, disabled, count, inline_checkboxes]() -> DrawResult {
			DrawResult result;
			
			for (size_t i = 0; i < count; ++i)
			{
				if (auto d = DisabledSection(disabled && disabled[i]))
				{
					result |= gui::InputData<bool>{}(sublabels[i], data[i]);
					if (inline_checkboxes && i + 1 < count)
						ImGui::SameLine();
				}
			}

			return result;
		}));

		gui::PropertyGrid::SubmitRow();

		if (gui::PropertyGrid::Reset::AnyActivated())
		{
			for (size_t i = 0; i < count; ++i)
				data[i] = def[i];
		}
	}

	void DescIO::Draw(const char* label, EditSession<Rect>& data, const Rect& def)
	{
		gui::IDScope scope(&data);
		gui::PropertyGrid::Key::SetLabel(label);

		data.PreEdit();
		if (data.buffer != def)
			gui::PropertyGrid::Reset::Button();

		ValueLabelInputData<float>{}("x1", "##x1", data.buffer.x1);
		ValueLabelInputData<float>{}("x2", "##x2", data.buffer.x2);
		ValueLabelInputData<float>{}("y1", "##y1", data.buffer.y1);
		ValueLabelInputData<float>{}("y2", "##y2", data.buffer.y2);

		gui::PropertyGrid::SubmitRow();
		data.PostEdit(gui::PropertyGrid::Value::GetDrawResult());
		if (gui::PropertyGrid::Reset::AnyActivated())
			data.PublishReset(def);
	}
	
	void DescIO::Draw(const char* label, EditSession<UVRect>& data, const UVRect& def)
	{
		gui::IDScope scope(&data);
		gui::PropertyGrid::Key::SetLabel(label);

		data.PreEdit();
		if (data.buffer != def)
			gui::PropertyGrid::Reset::Button();

		ValueLabelInputData<float>{}("x1", "##x1", data.buffer.x1, MakeOpt(0.f), MakeOpt(1.f));
		ValueLabelInputData<float>{}("x2", "##x2", data.buffer.x2, MakeOpt(0.f), MakeOpt(1.f));
		ValueLabelInputData<float>{}("y1", "##y1", data.buffer.y1, MakeOpt(0.f), MakeOpt(1.f));
		ValueLabelInputData<float>{}("y2", "##y2", data.buffer.y2, MakeOpt(0.f), MakeOpt(1.f));

		gui::PropertyGrid::SubmitRow();
		data.PostEdit(gui::PropertyGrid::Value::GetDrawResult());
		if (gui::PropertyGrid::Reset::AnyActivated())
			data.PublishReset(def);
	}
	
	void DescIO::Draw(const char* label, EditSession<TopSidePadding>& data, const TopSidePadding& def)
	{
		gui::IDScope scope(&data);
		gui::PropertyGrid::Key::SetLabel(label);

		data.PreEdit();
		if (data.buffer != def)
			gui::PropertyGrid::Reset::Button();

		ValueLabelInputData<float>{}("left", "##left", data.buffer.left);
		ValueLabelInputData<float>{}("right", "##right", data.buffer.right);
		ValueLabelInputData<float>{}("top", "##top", data.buffer.top);

		gui::PropertyGrid::SubmitRow();
		data.PostEdit(gui::PropertyGrid::Value::GetDrawResult());
		if (gui::PropertyGrid::Reset::AnyActivated())
			data.PublishReset(def);
	}

	template<>
	DrawResult DescIO::DrawCombo(const char* label, MemoryUnit& data)
	{
		return DrawEnumCombo(label, data, { "B", "KB", "KiB", "MB", "MiB", "GB", "GiB" });
	}

	template<>
	DrawResult DescIO::DrawCombo(const char* label, detail::Axis0dConversion& data)
	{
		return DrawEnumCombo(label, data, { "None", "To 1D", "To 2D", "To 3D" });
	}

	template<>
	DrawResult DescIO::DrawCombo(const char* label, detail::Axis1dConversion& data)
	{
		return DrawEnumCombo(label, data, { "None", "To 0D", "To 2D", "To 3D" });
	}

	template<>
	DrawResult DescIO::DrawCombo(const char* label, detail::Axis2dConversion& data)
	{
		return DrawEnumCombo(label, data, { "None", "To 0D (X)", "To 0D (Y)", "To 0D (XY)", "To 1D (X)", "To 1D (Y)", "To 1D (XY)", "To 3D (z=0)", "To 3D (z=1)" });
	}

	template<>
	DrawResult DescIO::DrawCombo(const char* label, detail::CommonBufferPreset& data)
	{
		return DrawEnumCombo(label, data, { "Common", "Alphanumeric", "Numeric", "Alphabet", "Alphabet (lowercase)", "Alphabet (uppercase)" });
	}

	template<>
	DrawResult DescIO::DrawCombo(const char* label, detail::GamepadAxis2D& data)
	{
		return DrawEnumCombo(label, data, { "Left XY", "Right XY" });
	}

	template<>
	DrawResult DescIO::DrawCombo(const char* label, detail::PositioningMode& data)
	{
		return DrawEnumCombo(label, data, { "Relative", "Absolute" });
	}

	template<>
	DrawResult DescIO::DrawCombo(const char* label, detail::SignalBindingType& data)
	{
		return DrawEnumCombo(label, data, { "Key", "Mouse Button", "Gamepad Button", "Gamepad Axis 1D", "Gamepad Axis 2D", "Cursor Position", "Scroll" });
	}

	template<>
	DrawResult DescIO::DrawCombo(const char* label, detail::SpritesheetParamType& data)
	{
		return DrawEnumCombo(label, data, { "Index", "Pixel" });
	}

	template<>
	DrawResult DescIO::DrawCombo(const char* label, detail::StorageMode& data)
	{
		return DrawEnumCombo(label, data, { "Discard", "Keep" });
	}

	template<>
	DrawResult DescIO::DrawCombo(const char* label, detail::Swizzle& data)
	{
		return DrawEnumCombo(label, data, { "None", "YX", "XZY", "YXZ", "YZX", "ZXY", "ZYX" });
	}

	template<>
	DrawResult DescIO::DrawCombo(const char* label, detail::SVGMipmapGenerationMode& data)
	{
		return DrawEnumCombo(label, data, { "Auto", "Off", "Manual" });
	}

	template<>
	DrawResult DescIO::DrawCombo(const char* label, detail::TileRotation& data)
	{
		return DrawEnumCombo(label, data, { "None", "90 degrees", "180 degrees", "270 degrees" });
	}
}
