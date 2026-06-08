#pragma once

#include "desc/Fields.h"
#include "desc/ArrayDesc.h"

#include "definitions/enums/AxisConversions.h"
#include "definitions/enums/GamepadAxis2D.h"
#include "definitions/enums/SignalBindingType.h"

namespace oly::editor
{
#define MODIFIER_BASE_GENERATOR(M) \
		M(swizzle) \
		M(invert)
		//M(multiplier) \
		//M(invert)

	struct ModifierBaseDesc
	{
		EnumField<detail::Swizzle> swizzle;
		//Vec3Field<MakeOpt<float>(), MakeOpt<float>()> multiplier;
		BoolArrayField<3> invert;

		ModifierBaseDesc();

		DESC_CHAIN_METHODS(ModifierBaseDesc, MODIFIER_BASE_GENERATOR);
	};

#define MODIFIER_0D_GENERATOR(M) \
		M(base) \
		M(conversion)

	struct Modifier0dDesc
	{
		ModifierBaseDesc base;
		EnumField<detail::Axis0dConversion> conversion;

		Modifier0dDesc();
		
		DESC_CHAIN_METHODS(Modifier0dDesc, MODIFIER_0D_GENERATOR);
	};

#define MODIFIER_1D_GENERATOR(M) \
		M(base) \
		M(conversion)

	struct Modifier1dDesc
	{
		ModifierBaseDesc base;
		EnumField<detail::Axis1dConversion> conversion;

		Modifier1dDesc();

		DESC_CHAIN_METHODS(Modifier1dDesc, MODIFIER_1D_GENERATOR);
	};

#define MODIFIER_2D_GENERATOR(M) \
		M(base) \
		M(conversion)

	struct Modifier2dDesc
	{
		ModifierBaseDesc base;
		EnumField<detail::Axis2dConversion> conversion;

		Modifier2dDesc();

		DESC_CHAIN_METHODS(Modifier2dDesc, MODIFIER_2D_GENERATOR);
	};

#define KEY_GENERATOR(M) \
		M(modifier)

	struct KeyDesc
	{
		// TODO v8 key, required_mods, forbidden_mods
		Modifier0dDesc modifier;

		KeyDesc();

		DESC_CHAIN_METHODS(KeyDesc, KEY_GENERATOR);
	};

#define MOUSE_BUTTON_GENERATOR(M) \
		M(modifier)

	struct MouseButtonDesc
	{
		// TODO v8 button, required_mods, forbidden_mods
		Modifier0dDesc modifier;

		MouseButtonDesc();

		DESC_CHAIN_METHODS(MouseButtonDesc, MOUSE_BUTTON_GENERATOR);
	};

#define GAMEPAD_BUTTON_GENERATOR(M) \
		M(button) \
		M(modifier)

	struct GamepadButtonDesc
	{
		GLenumField button;
		Modifier0dDesc modifier;

		GamepadButtonDesc();

		DESC_CHAIN_METHODS(GamepadButtonDesc, GAMEPAD_BUTTON_GENERATOR);
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

		DESC_CHAIN_METHODS(GamepadAxis1dDesc, GAMEPAD_AXIS_1D_GENERATOR);
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

		DESC_CHAIN_METHODS(GamepadAxis2dDesc, GAMEPAD_AXIS_2D_GENERATOR);
	};

#define CURSOR_POS_GENERATOR(M) \
		M(modifier)

	struct CursorPosDesc
	{
		Modifier2dDesc modifier;

		DESC_CHAIN_METHODS(CursorPosDesc, CURSOR_POS_GENERATOR);
	};

#define SCROLL_GENERATOR(M) \
		M(modifier)

	struct ScrollDesc
	{
		Modifier2dDesc modifier;

		DESC_CHAIN_METHODS(ScrollDesc, SCROLL_GENERATOR);
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

		DESC_CHAIN_METHODS(SignalDesc, SIGNAL_GENERATOR);
	};

#define ROUTE_GENERATOR(M) \
	M(id) \
	//M(signals)

	struct RouteDesc
	{
		StringField id;
		//StringListField signals;

		RouteDesc();

		DESC_CHAIN_METHODS(RouteDesc, ROUTE_GENERATOR);
	};

#define SIGNAL_FULL_GENERATOR(M) \
	M(signals) \
	M(routes)

	struct SignalFullDesc
	{
		ArrayDesc<SignalDesc> signals;
		static const detail::Key signals_key;
		ArrayDesc<RouteDesc> routes;
		static const detail::Key routes_key;
		
		DESC_CHAIN_METHODS(SignalFullDesc, SIGNAL_FULL_GENERATOR);
	};
}
