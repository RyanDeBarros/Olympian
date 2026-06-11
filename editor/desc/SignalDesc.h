#pragma once

#include "desc/Fields.h"
#include "desc/Descriptors.h"

#include "definitions/enums/AxisConversions.h"
#include "definitions/enums/GamepadAxis2D.h"
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
		EnumField<detail::Swizzle> swizzle;
		Vec3Field<MakeOpt<float>(), MakeOpt<float>()> multiplier;
		BoolArrayField<3> invert;

		ModifierBaseDesc();
	};

#define MODIFIER_0D_PARTIAL_GENERATOR(M) \
		M(conversion)

#define MODIFIER_0D_GENERATOR(M) \
		M(base) \
		MODIFIER_0D_PARTIAL_GENERATOR(M)

	struct Modifier0dDesc
	{
		ModifierBaseDesc base;
		EnumField<detail::Axis0dConversion> conversion;

		Modifier0dDesc();
	};

#define MODIFIER_1D_PARTIAL_GENERATOR(M) \
		M(conversion)

#define MODIFIER_1D_GENERATOR(M) \
		M(base) \
		MODIFIER_1D_PARTIAL_GENERATOR(M)

	struct Modifier1dDesc
	{
		ModifierBaseDesc base;
		EnumField<detail::Axis1dConversion> conversion;

		Modifier1dDesc();
	};

#define MODIFIER_2D_PARTIAL_GENERATOR(M) \
		M(conversion)

#define MODIFIER_2D_GENERATOR(M) \
		M(base) \
		MODIFIER_2D_PARTIAL_GENERATOR(M)

	struct Modifier2dDesc
	{
		ModifierBaseDesc base;
		EnumField<detail::Axis2dConversion> conversion;

		Modifier2dDesc();
	};

#define KEY_PARTIAL_GENERATOR(M)

#define KEY_GENERATOR(M) \
		KEY_PARTIAL_GENERATOR(M) \
		M(modifier)

	struct KeyDesc
	{
		// TODO v8 key, required_mods, forbidden_mods
		Modifier0dDesc modifier;

		KeyDesc();
	};

#define MOUSE_BUTTON_PARTIAL_GENERATOR(M) \
		M(button)

#define MOUSE_BUTTON_GENERATOR(M) \
		MOUSE_BUTTON_PARTIAL_GENERATOR(M) \
		M(modifier)

	struct MouseButtonDesc
	{
		DisjointEnumField<detail::MouseButton> button;
		// TODO v8 required_mods, forbidden_mods
		Modifier0dDesc modifier;

		MouseButtonDesc();
	};

#define GAMEPAD_BUTTON_PARTIAL_GENERATOR(M) \
		M(button)

#define GAMEPAD_BUTTON_GENERATOR(M) \
		GAMEPAD_BUTTON_PARTIAL_GENERATOR(M) \
		M(modifier)

	struct GamepadButtonDesc
	{
		DisjointEnumField<GLenum> button;
		Modifier0dDesc modifier;

		GamepadButtonDesc();
	};

#define GAMEPAD_AXIS_1D_PARTIAL_GENERATOR(M) \
		M(axis) \
		M(deadzone)

#define GAMEPAD_AXIS_1D_GENERATOR(M) \
		GAMEPAD_AXIS_1D_PARTIAL_GENERATOR(M) \
		M(modifier)

	struct GamepadAxis1DDesc
	{
		DisjointEnumField<GLenum> axis;
		Modifier1dDesc modifier;
		FloatField<MakeOpt(0.f), MakeOpt(1.f)> deadzone;

		GamepadAxis1DDesc();
	};

#define GAMEPAD_AXIS_2D_PARTIAL_GENERATOR(M) \
		M(axis) \
		M(deadzone)

#define GAMEPAD_AXIS_2D_GENERATOR(M) \
		GAMEPAD_AXIS_2D_PARTIAL_GENERATOR(M) \
		M(modifier)

	struct GamepadAxis2DDesc
	{
		EnumField<detail::GamepadAxis2D> axis;
		Modifier2dDesc modifier;
		FloatField<MakeOpt(0.f), MakeOpt(1.f)> deadzone;

		GamepadAxis2DDesc();
	};

#define CURSOR_POS_PARTIAL_GENERATOR(M)

#define CURSOR_POS_GENERATOR(M) \
		CURSOR_POS_PARTIAL_GENERATOR(M) \
		M(modifier)

	struct CursorPosDesc
	{
		Modifier2dDesc modifier;
	};

#define SCROLL_PARTIAL_GENERATOR(M)

#define SCROLL_GENERATOR(M) \
		SCROLL_PARTIAL_GENERATOR(M) \
		M(modifier)

	struct ScrollDesc
	{
		Modifier2dDesc modifier;
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
		StringField id;
		EnumField<detail::SignalBindingType> binding;
		VariantDesc<KeyDesc, MouseButtonDesc, GamepadButtonDesc, GamepadAxis1DDesc, GamepadAxis2DDesc, CursorPosDesc, ScrollDesc> variant;
		static const detail::Key modifier_key;

		SignalDesc();
	};

#define ROUTE_GENERATOR(M) \
	M(id) \
	M(signals)

	struct RouteDesc
	{
		StringField id;
		StringVectorField signals;

		RouteDesc();
	};

#define SIGNAL_FULL_GENERATOR(M) \
	M(signals) \
	M(routes)

	struct SignalFullDesc
	{
		VectorDesc<SignalDesc> signals;
		static const detail::Key signals_key;
		VectorDesc<RouteDesc> routes;
		static const detail::Key routes_key;
	};
}
