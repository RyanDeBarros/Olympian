#pragma once

#include "desc/Fields.h"
#include "desc/Descriptors.h"

#include "definitions/enums/AxisConversions.h"
#include "definitions/enums/GamepadAxis2D.h"
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

#define MODIFIER_0D_GENERATOR(M) \
		M(base) \
		M(conversion)

	struct Modifier0dDesc
	{
		ModifierBaseDesc base;
		EnumField<detail::Axis0dConversion> conversion;

		Modifier0dDesc();
	};

#define MODIFIER_1D_GENERATOR(M) \
		M(base) \
		M(conversion)

	struct Modifier1dDesc
	{
		ModifierBaseDesc base;
		EnumField<detail::Axis1dConversion> conversion;

		Modifier1dDesc();
	};

#define MODIFIER_2D_GENERATOR(M) \
		M(base) \
		M(conversion)

	struct Modifier2dDesc
	{
		ModifierBaseDesc base;
		EnumField<detail::Axis2dConversion> conversion;

		Modifier2dDesc();
	};

#define KEY_GENERATOR(M) \
		M(modifier)

	struct KeyDesc
	{
		// TODO v8 key, required_mods, forbidden_mods
		Modifier0dDesc modifier;

		KeyDesc();
	};

#define MOUSE_BUTTON_GENERATOR(M) \
		M(modifier)

	struct MouseButtonDesc
	{
		// TODO v8 button, required_mods, forbidden_mods
		Modifier0dDesc modifier;

		MouseButtonDesc();
	};

#define GAMEPAD_BUTTON_GENERATOR(M) \
		M(button) \
		M(modifier)

	struct GamepadButtonDesc
	{
		GLenumField button;
		Modifier0dDesc modifier;

		GamepadButtonDesc();
	};

#define GAMEPAD_AXIS_1D_GENERATOR(M) \
		M(axis) \
		M(modifier) \
		M(deadzone)

	struct GamepadAxis1dDesc
	{
		GLenumField axis;
		Modifier1dDesc modifier;
		FloatField<MakeOpt(0.f), MakeOpt(1.f)> deadzone;

		GamepadAxis1dDesc();
	};

#define GAMEPAD_AXIS_2D_GENERATOR(M) \
		M(axis) \
		M(modifier) \
		M(deadzone)

	struct GamepadAxis2dDesc
	{
		EnumField<detail::GamepadAxis2D> axis;
		Modifier2dDesc modifier;
		FloatField<MakeOpt(0.f), MakeOpt(1.f)> deadzone;

		GamepadAxis2dDesc();
	};

#define CURSOR_POS_GENERATOR(M) \
		M(modifier)

	struct CursorPosDesc
	{
		Modifier2dDesc modifier;
	};

#define SCROLL_GENERATOR(M) \
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
		//M(variant)

	struct SignalDesc
	{
		StringField id;
		EnumField<detail::SignalBindingType> binding;
		//VariantDesc<KeyDesc, MouseButtonDesc, GamepadButtonDesc, GamepadAxis1dDesc, GamepadAxis2dDesc, CursorPosDesc, ScrollDesc> variant;
		static const detail::Key modifier_key;

		SignalDesc();
	};

#define ROUTE_GENERATOR(M) \
	M(id) \
	//M(signals)

	struct RouteDesc
	{
		StringField id;
		//StringListField signals;

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
