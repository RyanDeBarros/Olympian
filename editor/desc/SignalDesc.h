#pragma once

#include "bindings/Fields.h"

#include "definitions/enums/AxisConversions.h"
#include "definitions/enums/GamepadAxis2D.h"
#include "definitions/enums/InputMod.h"
#include "definitions/enums/KeyInput.h"
#include "definitions/enums/MouseButton.h"
#include "definitions/enums/SignalBindingType.h"

namespace oly::editor
{
#define MODIFIER_BASE_GENERATOR(M) \
		M((imtk::field::enum_fld<detail::Swizzle>), swizzle) \
		M((imtk::field::vec3_fld<imp::nullpotential, imp::nullpotential>), multiplier) \
		M((imtk::field::bool_array_fld<3>), invert)

	struct ModifierBaseDesc
	{
		IMTK_DESCRIPTOR_BODY(ModifierBaseDesc, MODIFIER_BASE_GENERATOR);

		ModifierBaseDesc(imtk::datapath_link link = {});
	};

#define MODIFIER_0D_PARTIAL_GENERATOR(M) \
		M((imtk::field::enum_fld<detail::Axis0dConversion>), conversion)

#define MODIFIER_0D_GENERATOR(M) \
		M((ModifierBaseDesc), base) \
		MODIFIER_0D_PARTIAL_GENERATOR(M)

	struct Modifier0dDesc
	{
		IMTK_DESCRIPTOR_BODY(Modifier0dDesc, MODIFIER_0D_GENERATOR);

		Modifier0dDesc(imtk::datapath_link link = {});
	};

#define MODIFIER_1D_PARTIAL_GENERATOR(M) \
		M((imtk::field::enum_fld<detail::Axis1dConversion>), conversion)

#define MODIFIER_1D_GENERATOR(M) \
		M((ModifierBaseDesc), base) \
		MODIFIER_1D_PARTIAL_GENERATOR(M)

	struct Modifier1dDesc
	{
		IMTK_DESCRIPTOR_BODY(Modifier1dDesc, MODIFIER_1D_GENERATOR);

		Modifier1dDesc(imtk::datapath_link link = {});
	};

#define MODIFIER_2D_PARTIAL_GENERATOR(M) \
		M((imtk::field::enum_fld<detail::Axis2dConversion>), conversion)

#define MODIFIER_2D_GENERATOR(M) \
		M((ModifierBaseDesc), base) \
		MODIFIER_2D_PARTIAL_GENERATOR(M)

	struct Modifier2dDesc
	{
		IMTK_DESCRIPTOR_BODY(Modifier2dDesc, MODIFIER_2D_GENERATOR);

		Modifier2dDesc(imtk::datapath_link link = {});
	};

#define KEY_PARTIAL_GENERATOR(M) \
		M((imtk::field::disjoint_enum_fld<detail::KeyInput>), key) \
		M((imtk::field::bitset_fld<detail::InputMod, detail::INPUT_MOD_COUNT>), required_mods) \
		M((imtk::field::bitset_fld<detail::InputMod, detail::INPUT_MOD_COUNT>), forbidden_mods)

#define KEY_GENERATOR(M) \
		KEY_PARTIAL_GENERATOR(M) \
		M((imtk::desc::sub<Modifier0dDesc>), modifier)

	struct KeyDesc
	{
		IMTK_DESCRIPTOR_BODY(KeyDesc, KEY_GENERATOR);

		KeyDesc(imtk::datapath_link link = {});
	};

#define MOUSE_BUTTON_PARTIAL_GENERATOR(M) \
		M((imtk::field::disjoint_enum_fld<detail::MouseButton>), button) \
		M((imtk::field::bitset_fld<detail::InputMod, detail::INPUT_MOD_COUNT>), required_mods) \
		M((imtk::field::bitset_fld<detail::InputMod, detail::INPUT_MOD_COUNT>), forbidden_mods)

#define MOUSE_BUTTON_GENERATOR(M) \
		MOUSE_BUTTON_PARTIAL_GENERATOR(M) \
		M((imtk::desc::sub<Modifier0dDesc>), modifier)

	struct MouseButtonDesc
	{
		IMTK_DESCRIPTOR_BODY(MouseButtonDesc, MOUSE_BUTTON_GENERATOR);

		MouseButtonDesc(imtk::datapath_link link = {});
	};

#define GAMEPAD_BUTTON_PARTIAL_GENERATOR(M) \
		M((imtk::field::disjoint_enum_fld<GLenum>), button)

#define GAMEPAD_BUTTON_GENERATOR(M) \
		GAMEPAD_BUTTON_PARTIAL_GENERATOR(M) \
		M((imtk::desc::sub<Modifier0dDesc>), modifier)

	struct GamepadButtonDesc
	{
		IMTK_DESCRIPTOR_BODY(GamepadButtonDesc, GAMEPAD_BUTTON_GENERATOR);

		GamepadButtonDesc(imtk::datapath_link link = {});
	};

#define GAMEPAD_AXIS_1D_PARTIAL_GENERATOR(M) \
		M((imtk::field::disjoint_enum_fld<GLenum>), axis) \
		M((imtk::field::float_fld<0.f, 1.f>), deadzone)

#define GAMEPAD_AXIS_1D_GENERATOR(M) \
		GAMEPAD_AXIS_1D_PARTIAL_GENERATOR(M) \
		M((imtk::desc::sub<Modifier1dDesc>), modifier)

	struct GamepadAxis1DDesc
	{
		IMTK_DESCRIPTOR_BODY(GamepadAxis1DDesc, GAMEPAD_AXIS_1D_GENERATOR);

		GamepadAxis1DDesc(imtk::datapath_link link = {});
	};

#define GAMEPAD_AXIS_2D_PARTIAL_GENERATOR(M) \
		M((imtk::field::enum_fld<detail::GamepadAxis2D>), axis) \
		M((imtk::field::float_fld<0.f, 1.f>), deadzone)

#define GAMEPAD_AXIS_2D_GENERATOR(M) \
		GAMEPAD_AXIS_2D_PARTIAL_GENERATOR(M) \
		M((imtk::desc::sub<Modifier2dDesc>), modifier)

	struct GamepadAxis2DDesc
	{
		IMTK_DESCRIPTOR_BODY(GamepadAxis2DDesc, GAMEPAD_AXIS_2D_GENERATOR);

		GamepadAxis2DDesc(imtk::datapath_link link = {});
	};

#define CURSOR_POS_PARTIAL_GENERATOR(M)

#define CURSOR_POS_GENERATOR(M) \
		CURSOR_POS_PARTIAL_GENERATOR(M) \
		M((imtk::desc::sub<Modifier2dDesc>), modifier)

	struct CursorPosDesc
	{
		IMTK_DESCRIPTOR_BODY(CursorPosDesc, CURSOR_POS_GENERATOR);

		CursorPosDesc(imtk::datapath_link link = {});
	};

#define SCROLL_PARTIAL_GENERATOR(M)

#define SCROLL_GENERATOR(M) \
		SCROLL_PARTIAL_GENERATOR(M) \
		M((imtk::desc::sub<Modifier2dDesc>), modifier)

	struct ScrollDesc
	{
		IMTK_DESCRIPTOR_BODY(ScrollDesc, SCROLL_GENERATOR);

		ScrollDesc(imtk::datapath_link link = {});
	};

#define BINDING_TYPE_GENERATOR(M) \
		M(Key) \
		M(MouseButton) \
		M(GamepadButton) \
		M(GamepadAxis1D) \
		M(GamepadAxis2D) \
		M(CursorPos) \
		M(Scroll)

#define SIGNAL_PARTIAL_GENERATOR(M) \
		M((imtk::field::string_fld), id) \
		M((imtk::field::enum_fld<detail::SignalBindingType>), binding)

#define SIGNAL_GENERATOR(M) \
		SIGNAL_PARTIAL_GENERATOR(M) \
		M((imtk::desc::variant<KeyDesc, MouseButtonDesc, GamepadButtonDesc, GamepadAxis1DDesc, GamepadAxis2DDesc, CursorPosDesc, ScrollDesc>), variant)

	struct SignalDesc
	{
		IMTK_DESCRIPTOR_BODY(SignalDesc, SIGNAL_GENERATOR);

		SignalDesc(imtk::datapath_link link = {});
	};

#define ROUTE_GENERATOR(M) \
	M((imtk::field::string_fld), id) \
	M((imtk::field::string_vector_fld), signals)

	struct RouteDesc
	{
		IMTK_DESCRIPTOR_BODY(RouteDesc, ROUTE_GENERATOR);

		RouteDesc(imtk::datapath_link link = {});
	};

#define SIGNAL_FULL_GENERATOR(M) \
	M((imtk::desc::vector<SignalDesc>), signals) \
	M((imtk::desc::vector<RouteDesc>), routes)

	struct SignalFullDesc
	{
		IMTK_DESCRIPTOR_BODY(SignalFullDesc, SIGNAL_FULL_GENERATOR);

		SignalFullDesc(imtk::datapath_link link = {});
	};
}
