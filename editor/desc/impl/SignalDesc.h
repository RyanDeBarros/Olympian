#pragma once

#include "desc/Fields.h"

#include "definitions/enums/AxisConversions.h"
#include "definitions/enums/GamepadAxis2D.h"
#include "definitions/enums/InputMod.h"
#include "definitions/enums/KeyInput.h"
#include "definitions/enums/MouseButton.h"
#include "definitions/enums/SignalBindingType.h"

namespace oly::editor
{
#define MODIFIER_BASE_GENERATOR(M) \
		M(swizzle) \
		M(multiplier) \
		M(invert)

	struct ModifierBaseDesc
	{
		IMTK_DESCRIPTOR_BODY(ModifierBaseDesc, MODIFIER_BASE_GENERATOR);

		EnumField<detail::Swizzle> swizzle;
		Vec3Field<MakeOpt<float>(), MakeOpt<float>()> multiplier;
		BoolArrayField<3> invert;

		ModifierBaseDesc(imtk::datapath_link link = {});
	};

#define MODIFIER_0D_PARTIAL_GENERATOR(M) \
		M(conversion)

#define MODIFIER_0D_GENERATOR(M) \
		M(base) \
		MODIFIER_0D_PARTIAL_GENERATOR(M)

	struct Modifier0dDesc
	{
		IMTK_DESCRIPTOR_BODY(Modifier0dDesc, MODIFIER_0D_GENERATOR);

		ModifierBaseDesc base;
		EnumField<detail::Axis0dConversion> conversion;

		Modifier0dDesc(imtk::datapath_link link = {});
	};

#define MODIFIER_1D_PARTIAL_GENERATOR(M) \
		M(conversion)

#define MODIFIER_1D_GENERATOR(M) \
		M(base) \
		MODIFIER_1D_PARTIAL_GENERATOR(M)

	struct Modifier1dDesc
	{
		IMTK_DESCRIPTOR_BODY(Modifier1dDesc, MODIFIER_1D_GENERATOR);

		ModifierBaseDesc base;
		EnumField<detail::Axis1dConversion> conversion;

		Modifier1dDesc(imtk::datapath_link link = {});
	};

#define MODIFIER_2D_PARTIAL_GENERATOR(M) \
		M(conversion)

#define MODIFIER_2D_GENERATOR(M) \
		M(base) \
		MODIFIER_2D_PARTIAL_GENERATOR(M)

	struct Modifier2dDesc
	{
		IMTK_DESCRIPTOR_BODY(Modifier2dDesc, MODIFIER_2D_GENERATOR);

		ModifierBaseDesc base;
		EnumField<detail::Axis2dConversion> conversion;

		Modifier2dDesc(imtk::datapath_link link = {});
	};

#define KEY_MODS_GENERATOR(M) \
		M(required_mods) \
		M(forbidden_mods)

#define KEY_PARTIAL_GENERATOR(M) \
		M(key) \
		KEY_MODS_GENERATOR(M)

#define KEY_GENERATOR(M) \
		KEY_PARTIAL_GENERATOR(M) \
		M(modifier)

	struct KeyDesc
	{
		IMTK_DESCRIPTOR_BODY(KeyDesc, KEY_GENERATOR);

		DisjointEnumField<detail::KeyInput> key;
		BitsetField<detail::InputMod, detail::INPUT_MOD_COUNT> required_mods;
		BitsetField<detail::InputMod, detail::INPUT_MOD_COUNT> forbidden_mods;
		Modifier0dDesc modifier;

		KeyDesc(imtk::datapath_link link = {});
	};

#define MOUSE_BUTTON_MODS_GENERATOR(M) \
		M(required_mods) \
		M(forbidden_mods)

#define MOUSE_BUTTON_PARTIAL_GENERATOR(M) \
		M(button) \
		MOUSE_BUTTON_MODS_GENERATOR(M)

#define MOUSE_BUTTON_GENERATOR(M) \
		MOUSE_BUTTON_PARTIAL_GENERATOR(M) \
		M(modifier)

	struct MouseButtonDesc
	{
		IMTK_DESCRIPTOR_BODY(MouseButtonDesc, MOUSE_BUTTON_GENERATOR);

		DisjointEnumField<detail::MouseButton> button;
		BitsetField<detail::InputMod, detail::INPUT_MOD_COUNT> required_mods;
		BitsetField<detail::InputMod, detail::INPUT_MOD_COUNT> forbidden_mods;
		Modifier0dDesc modifier;

		MouseButtonDesc(imtk::datapath_link link = {});
	};

#define GAMEPAD_BUTTON_PARTIAL_GENERATOR(M) \
		M(button)

#define GAMEPAD_BUTTON_GENERATOR(M) \
		GAMEPAD_BUTTON_PARTIAL_GENERATOR(M) \
		M(modifier)

	struct GamepadButtonDesc
	{
		IMTK_DESCRIPTOR_BODY(GamepadButtonDesc, GAMEPAD_BUTTON_GENERATOR);

		DisjointEnumField<GLenum> button;
		Modifier0dDesc modifier;

		GamepadButtonDesc(imtk::datapath_link link = {});
	};

#define GAMEPAD_AXIS_1D_PARTIAL_GENERATOR(M) \
		M(axis) \
		M(deadzone)

#define GAMEPAD_AXIS_1D_GENERATOR(M) \
		GAMEPAD_AXIS_1D_PARTIAL_GENERATOR(M) \
		M(modifier)

	struct GamepadAxis1DDesc
	{
		IMTK_DESCRIPTOR_BODY(GamepadAxis1DDesc, GAMEPAD_AXIS_1D_GENERATOR);

		DisjointEnumField<GLenum> axis;
		Modifier1dDesc modifier;
		FloatField<MakeOpt(0.f), MakeOpt(1.f)> deadzone;

		GamepadAxis1DDesc(imtk::datapath_link link = {});
	};

#define GAMEPAD_AXIS_2D_PARTIAL_GENERATOR(M) \
		M(axis) \
		M(deadzone)

#define GAMEPAD_AXIS_2D_GENERATOR(M) \
		GAMEPAD_AXIS_2D_PARTIAL_GENERATOR(M) \
		M(modifier)

	struct GamepadAxis2DDesc
	{
		IMTK_DESCRIPTOR_BODY(GamepadAxis2DDesc, GAMEPAD_AXIS_2D_GENERATOR);

		EnumField<detail::GamepadAxis2D> axis;
		Modifier2dDesc modifier;
		FloatField<MakeOpt(0.f), MakeOpt(1.f)> deadzone;

		GamepadAxis2DDesc(imtk::datapath_link link = {});
	};

#define CURSOR_POS_PARTIAL_GENERATOR(M)

#define CURSOR_POS_GENERATOR(M) \
		CURSOR_POS_PARTIAL_GENERATOR(M) \
		M(modifier)

	struct CursorPosDesc
	{
		IMTK_DESCRIPTOR_BODY(CursorPosDesc, CURSOR_POS_GENERATOR);

		Modifier2dDesc modifier;

		CursorPosDesc(imtk::datapath_link link = {});
	};

#define SCROLL_PARTIAL_GENERATOR(M)

#define SCROLL_GENERATOR(M) \
		SCROLL_PARTIAL_GENERATOR(M) \
		M(modifier)

	struct ScrollDesc
	{
		IMTK_DESCRIPTOR_BODY(ScrollDesc, SCROLL_GENERATOR);
		
		Modifier2dDesc modifier;

		ScrollDesc(imtk::datapath_link link = {});
	};

#define SIGNAL_PARTIAL_GENERATOR(M) \
		M(id) \
		M(binding)

#define SIGNAL_GENERATOR(M) \
		SIGNAL_PARTIAL_GENERATOR(M) \
		M(variant)

#define BINDING_TYPE_GENERATOR(M) \
		M(Key) \
		M(MouseButton) \
		M(GamepadButton) \
		M(GamepadAxis1D) \
		M(GamepadAxis2D) \
		M(CursorPos) \
		M(Scroll)

	struct SignalDesc
	{
		IMTK_DESCRIPTOR_BODY(SignalDesc, SIGNAL_GENERATOR);

		StringField id;
		EnumField<detail::SignalBindingType> binding;
		imtk::desc::variant<KeyDesc, MouseButtonDesc, GamepadButtonDesc, GamepadAxis1DDesc, GamepadAxis2DDesc, CursorPosDesc, ScrollDesc> variant;
		static const detail::Key modifier_key;

		SignalDesc(imtk::datapath_link link = {});
	};

#define ROUTE_GENERATOR(M) \
	M(id) \
	M(signals)

	struct RouteDesc
	{
		IMTK_DESCRIPTOR_BODY(RouteDesc, ROUTE_GENERATOR);

		StringField id;
		StringVectorField signals;

		RouteDesc(imtk::datapath_link link = {});
	};

#define SIGNAL_FULL_GENERATOR(M) \
	M(signals) \
	M(routes)

	struct SignalFullDesc
	{
		IMTK_DESCRIPTOR_BODY(SignalFullDesc, SIGNAL_FULL_GENERATOR);

		imtk::desc::vector<SignalDesc> signals;
		static const detail::Key signals_key;
		imtk::desc::vector<RouteDesc> routes;
		static const detail::Key routes_key;

		SignalFullDesc(imtk::datapath_link link = {});
	};
}
