#include "DescIO.h"

#include "gui/scopes/Subform.h"

#include "core/MemoryUnit.h"

#include "definitions/Keys.h"
#include "definitions/enums/Include.h"

#include <imgui.h>

#include <span>

// TODO DEBT support more complex property views. For example, a dynamic list of strings should be able to paste into another, even though they might have different sizes. Another example is dynamic descriptors, such as checkboxes or combos enabling/disabling sections.

namespace oly::editor
{
	void DescIO::Draw(const char* label, int& data, const int& def, imtk::label_span_registry::handle names)
	{
		imtk::id_scope scope(&data);
		imtk::prop::key::set_label(label);
		imtk::prop::value::add_component(std::make_unique<imtk::w::combo_widget>(data, names));
		if (data != def)
			imtk::prop::reset::button();
		imtk::prop::row::submit();
		if (imtk::prop::reset::any_activated())
			data = def;
	}

	void DescIO::Draw(const char* label, imtk::edit_session<std::string>* data, const std::string* def, size_t count)
	{
		imtk::prop::view_generator generator = [data, count]() {
			auto view = std::make_unique<imtk::prop::view_list>();
			for (size_t i = 0; i < count; ++i)
				view->subviews.push_back(std::make_unique<imtk::prop::simple_view<std::string>>(data[i].buffer()));
			return view;
		};

		if (auto subform = Subform(label, generator))
		{
			for (size_t i = 0; i < count; ++i)
				RowInputData(std::to_string(i).c_str(), data[i], def[i]);
		}
	}

	void DescIO::Draw(const char* label, imtk::edit_session<std::string>* data, const std::string* def, const char** sublabels, size_t count)
	{
		imtk::prop::view_generator generator = [data, count]() {
			auto view = std::make_unique<imtk::prop::view_list>();
			for (size_t i = 0; i < count; ++i)
				view->subviews.push_back(std::make_unique<imtk::prop::simple_view<std::string>>(data[i].buffer()));
			return view;
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
		imtk::id_scope scope(data);
		imtk::prop::key::set_label(label);

		for (size_t i = 0; i < count; ++i)
		{
			if (data[i] != def[i])
			{
				imtk::prop::reset::button();
				break;
			}
		}

		imtk::prop::value::add_component(std::make_unique<imtk::w::generic_widget>([&data, sublabels, disabled, count, inline_checkboxes]() -> imtk::item_result {
			imtk::item_result result;
			
			for (size_t i = 0; i < count; ++i)
			{
				if (auto d = imtk::disabled(disabled && disabled[i]))
				{
					result |= imtk::w::simple_widget<bool>(data[i], { .label = sublabels[i] }).draw();
					if (inline_checkboxes && i + 1 < count)
						ImGui::SameLine();
				}
			}

			return result;
		}));

		imtk::prop::row::submit();

		if (imtk::prop::reset::any_activated())
		{
			for (size_t i = 0; i < count; ++i)
				data[i] = def[i];
		}
	}

	void DescIO::Draw(const char* label, imtk::edit_session<Rect>& data, const Rect& def)
	{
		imtk::id_scope scope(&data);
		imtk::prop::key::set_label(label);

		data.pre_edit();
		if (data.buffer() != def)
			imtk::prop::reset::button();

		ValueInputData<float>{}("x1", data.buffer().x1);
		ValueInputDataSep<float>{}("x2", data.buffer().x2);
		ValueInputDataSep<float>{}("y1", data.buffer().y1);
		ValueInputDataSep<float>{}("y2", data.buffer().y2);

		imtk::prop::row::submit();
		data.post_edit(imtk::prop::value::get_draw_result().state);
		if (imtk::prop::reset::any_activated())
			data.publish_reset(def);
	}
	
	void DescIO::Draw(const char* label, imtk::edit_session<UVRect>& data, const UVRect& def)
	{
		imtk::id_scope scope(&data);
		imtk::prop::key::set_label(label);

		data.pre_edit();
		if (data.buffer() != def)
			imtk::prop::reset::button();

		ValueInputData<float>{}("x1", data.buffer().x1, imp::potential<float>(0.f), imp::potential<float>(1.f));
		ValueInputDataSep<float>{}("x2", data.buffer().x2, imp::potential<float>(0.f), imp::potential<float>(1.f));
		ValueInputDataSep<float>{}("y1", data.buffer().y1, imp::potential<float>(0.f), imp::potential<float>(1.f));
		ValueInputDataSep<float>{}("y2", data.buffer().y2, imp::potential<float>(0.f), imp::potential<float>(1.f));

		imtk::prop::row::submit();
		data.post_edit(imtk::prop::value::get_draw_result().state);
		if (imtk::prop::reset::any_activated())
			data.publish_reset(def);
	}
	
	void DescIO::Draw(const char* label, imtk::edit_session<TopSidePadding>& data, const TopSidePadding& def)
	{
		imtk::id_scope scope(&data);
		imtk::prop::key::set_label(label);

		data.pre_edit();
		if (data.buffer() != def)
			imtk::prop::reset::button();

		ValueInputData<float>{}("left", data.buffer().left);
		ValueInputDataSep<float>{}("right", data.buffer().right);
		ValueInputDataSep<float>{}("top", data.buffer().top);

		imtk::prop::row::submit();
		data.post_edit(imtk::prop::value::get_draw_result().state);
		if (imtk::prop::reset::any_activated())
			data.publish_reset(def);
	}

	template<>
	imtk::item_result DescIO::DrawCombo(MemoryUnit& data)
	{
		return DrawEnumCombo(data, { "B", "KB", "KiB", "MB", "MiB", "GB", "GiB" });
	}

	template<>
	imtk::item_result DescIO::DrawCombo(detail::Axis0dConversion& data)
	{
		return DrawEnumCombo(data, { "None", "To 1D", "To 2D", "To 3D" });
	}

	template<>
	imtk::item_result DescIO::DrawCombo(detail::Axis1dConversion& data)
	{
		return DrawEnumCombo(data, { "None", "To 0D", "To 2D", "To 3D" });
	}

	template<>
	imtk::item_result DescIO::DrawCombo(detail::Axis2dConversion& data)
	{
		return DrawEnumCombo(data, { "None", "To 0D (X)", "To 0D (Y)", "To 0D (XY)", "To 1D (X)", "To 1D (Y)", "To 1D (XY)", "To 3D (z=0)", "To 3D (z=1)" });
	}

	template<>
	imtk::item_result DescIO::DrawCombo(detail::CommonBufferPreset& data)
	{
		return DrawEnumCombo(data, { "Common", "Alphanumeric", "Numeric", "Alphabet", "Alphabet (lowercase)", "Alphabet (uppercase)" });
	}

	template<>
	imtk::item_result DescIO::DrawCombo(detail::GamepadAxis2D& data)
	{
		return DrawEnumCombo(data, { "Left XY", "Right XY" });
	}

	template<>
	imtk::item_result DescIO::DrawCombo(detail::PositioningMode& data)
	{
		return DrawEnumCombo(data, { "Relative", "Absolute" });
	}

	template<>
	imtk::item_result DescIO::DrawCombo(detail::SignalBindingType& data)
	{
		return DrawEnumCombo(data, { "Key", "Mouse Button", "Gamepad Button", "Gamepad Axis 1D", "Gamepad Axis 2D", "Cursor Position", "Scroll" });
	}

	template<>
	imtk::item_result DescIO::DrawCombo(detail::SpritesheetParamType& data)
	{
		return DrawEnumCombo(data, { "Index", "Pixel" });
	}

	template<>
	imtk::item_result DescIO::DrawCombo(detail::StorageMode& data)
	{
		return DrawEnumCombo(data, { "Discard", "Keep" });
	}

	template<>
	imtk::item_result DescIO::DrawCombo(detail::Swizzle& data)
	{
		return DrawEnumCombo(data, { "None", "YX", "XZY", "YXZ", "YZX", "ZXY", "ZYX" });
	}

	template<>
	imtk::item_result DescIO::DrawCombo(detail::SVGMipmapGenerationMode& data)
	{
		return DrawEnumCombo(data, { "Auto", "Off", "Manual" });
	}

	template<>
	imtk::item_result DescIO::DrawCombo(detail::TileRotation& data)
	{
		return DrawEnumCombo(data, { "None", "90 degrees", "180 degrees", "270 degrees" });
	}
}
