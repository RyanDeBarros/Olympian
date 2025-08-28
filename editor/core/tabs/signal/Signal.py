from __future__ import annotations

from dataclasses import dataclass, field, fields
from enum import IntEnum
from typing import Optional

from editor.core import GLFW_MOD_CONTROL, GLFW_MOD_SHIFT, GLFW_MOD_ALT, GLFW_MOD_SUPER, GLFW_MOD_CAPS_LOCK, \
	GLFW_MOD_NUM_LOCK


class InputType(IntEnum):
	KEY = 0
	MOUSE_BUTTON = 1
	GAMEPAD_BUTTON = 2
	GAMEPAD_1D_AXIS = 3
	GAMEPAD_2D_AXIS = 4
	CURSOR_POSITION = 5
	SCROLL = 6


class ModPolicy(IntEnum):
	IGNORE = 0,
	REQUIRED = 1,
	FORBIDDEN = 2


OLY_BINDING_NAMES = [
	'key',
	'mouse button',
	'gamepad button',
	'gamepad axis 1d',
	'gamepad axis 2d',
	'cursor pos',
	'scroll'
]

OLY_CONVERSION_0D_NAMES = [
	'TO_1D',
	'TO_2D',
	'TO_3D'
]

OLY_CONVERSION_1D_NAMES = [
	'TO_0D',
	'TO_2D',
	'TO_3D'
]

OLY_CONVERSION_2D_NAMES = [
	'TO_0D_X',
	'TO_0D_Y',
	'TO_0D_XY',
	'TO_1D_X',
	'TO_1D_Y',
	'TO_1D_XY',
	'TO_3D_0',
	'TO_3D_1'
]


@dataclass
class BasicSection:
	name: str
	type: InputType
	key: Optional[int] = None
	button: Optional[int] = None
	axis1d: Optional[int] = None
	axis2d: Optional[int] = None
	deadzone: Optional[float] = None


@dataclass
class ModSection:
	shift: int = 0
	ctrl: int = 0
	alt: int = 0
	super: int = 0
	caps_lock: int = 0
	num_lock: int = 0


@dataclass
class ConversionSection:
	dim: int = 0
	swizzle: str = 'None'
	multiplier: list[float] = field(default_factory=lambda: [1.0, 1.0, 1.0])
	invert: list[bool] = field(default_factory=lambda: [False, False, False])


@dataclass
class EditorSignal:
	basic: BasicSection
	conversion: ConversionSection = field(default_factory=lambda: ConversionSection())
	mods: Optional[ModSection] = None


@dataclass
class ModifierSection:
	swizzle: Optional[str] = None
	conversion: Optional[str] = None
	multiplier: Optional[list[float]] = None
	invert: Optional[list[bool]] = None

	@staticmethod
	def from_dict(d: dict) -> ModifierSection:
		init_values = {}
		for f in fields(ModifierSection):
			if f.name in d:
				init_values[f.name] = d[f.name]
		return ModifierSection(**init_values)

	def to_dict(self) -> dict:
		d = {}
		for key, value in self.__dict__.items():
			if hasattr(value, "to_dict"):
				d[key] = value.to_dict()
			else:
				d[key] = value
		return d


@dataclass
class OlySignal:
	id: str
	binding: str
	req_mods: Optional[int] = None
	ban_mods: Optional[int] = None
	key: Optional[int] = None
	button: Optional[int] = None
	axis1d: Optional[int] = None
	axis2d: Optional[int] = None
	deadzone: Optional[float] = None
	modifier: Optional[ModifierSection] = None

	@staticmethod
	def from_dict(d: dict) -> OlySignal:
		init_values = {}
		if d is not None:
			for f in fields(OlySignal):
				if f.name in d:
					match f.name:
						case 'modifier':
							init_values[f.name] = ModifierSection.from_dict(d[f.name])
						case _:
							init_values[f.name] = d[f.name]
		return OlySignal(**init_values)

	def to_dict(self) -> dict:
		d = {}
		for key, value in self.__dict__.items():
			if hasattr(value, "to_dict"):
				d[key] = value.to_dict()
			else:
				d[key] = value
		return d


def convert_signal_from_editor_to_oly_format(signal: EditorSignal) -> OlySignal:
	d = OlySignal(id=signal.basic.name, binding=OLY_BINDING_NAMES[signal.basic.type])

	match signal.basic.type:
		case 0:
			d.key = signal.basic.key
		case 1:
			d.button = signal.basic.button
		case 2:
			d.button = signal.basic.button
		case 3:
			d.axis1d = signal.basic.axis1d
			d.deadzone = signal.basic.deadzone
		case 4:
			d.axis2d = signal.basic.axis2d
			d.deadzone = signal.basic.deadzone

	if signal.mods is not None:
		d.req_mods = 0
		d.ban_mods = 0
		if signal.mods.shift == ModPolicy.REQUIRED:
			d.req_mods |= GLFW_MOD_SHIFT.value
		elif signal.mods.shift == ModPolicy.FORBIDDEN:
			d.ban_mods |= GLFW_MOD_SHIFT.value
		if signal.mods.ctrl == ModPolicy.REQUIRED:
			d.req_mods |= GLFW_MOD_CONTROL.value
		elif signal.mods.ctrl == ModPolicy.FORBIDDEN:
			d.ban_mods |= GLFW_MOD_CONTROL.value
		if signal.mods.alt == ModPolicy.REQUIRED:
			d.req_mods |= GLFW_MOD_ALT.value
		elif signal.mods.alt == ModPolicy.FORBIDDEN:
			d.ban_mods |= GLFW_MOD_ALT.value
		if signal.mods.super == ModPolicy.REQUIRED:
			d.req_mods |= GLFW_MOD_SUPER.value
		elif signal.mods.super == ModPolicy.FORBIDDEN:
			d.ban_mods |= GLFW_MOD_SUPER.value
		if signal.mods.caps_lock == ModPolicy.REQUIRED:
			d.req_mods |= GLFW_MOD_CAPS_LOCK.value
		elif signal.mods.caps_lock == ModPolicy.FORBIDDEN:
			d.ban_mods |= GLFW_MOD_CAPS_LOCK.value
		if signal.mods.num_lock == ModPolicy.REQUIRED:
			d.req_mods |= GLFW_MOD_NUM_LOCK.value
		elif signal.mods.num_lock == ModPolicy.FORBIDDEN:
			d.ban_mods |= GLFW_MOD_NUM_LOCK.value

	d.modifier = ModifierSection()
	if signal.conversion.swizzle != 'None':
		d.modifier.swizzle = signal.conversion.swizzle
	d.modifier.multiplier = signal.conversion.multiplier
	d.modifier.invert = signal.conversion.invert

	if signal.conversion.dim > 0:
		match signal.basic.type:
			case InputType.KEY | InputType.MOUSE_BUTTON | InputType.GAMEPAD_BUTTON:
				d.modifier.conversion = OLY_CONVERSION_0D_NAMES[signal.conversion.dim - 1]
			case InputType.GAMEPAD_1D_AXIS:
				d.modifier.conversion = OLY_CONVERSION_1D_NAMES[signal.conversion.dim - 1]
			case InputType.GAMEPAD_2D_AXIS | InputType.CURSOR_POSITION | InputType.SCROLL:
				d.modifier.conversion = OLY_CONVERSION_2D_NAMES[signal.conversion.dim - 1]

	return d


def convert_signal_from_oly_to_editor_format(signal: OlySignal) -> EditorSignal:
	d = EditorSignal(basic=BasicSection(name=signal.id, type=InputType(OLY_BINDING_NAMES.index(signal.binding))),
					 conversion=ConversionSection())

	match d.basic.type:
		case 0:
			d.basic.key = signal.key
		case 1:
			d.basic.button = signal.button
		case 2:
			d.basic.button = signal.button
		case 3:
			d.basic.axis1d = signal.axis1d
			d.basic.deadzone = signal.deadzone
		case 4:
			d.basic.axis2d = signal.axis2d
			d.basic.deadzone = signal.deadzone

	if d.basic.type in (InputType.KEY, InputType.MOUSE_BUTTON):
		d.mods = ModSection()

		if signal.req_mods is not None:
			if signal.req_mods & GLFW_MOD_SHIFT.value:
				d.mods.shift = ModPolicy.REQUIRED
			if signal.req_mods & GLFW_MOD_CONTROL.value:
				d.mods.ctrl = ModPolicy.REQUIRED
			if signal.req_mods & GLFW_MOD_ALT.value:
				d.mods.alt = ModPolicy.REQUIRED
			if signal.req_mods & GLFW_MOD_SUPER.value:
				d.mods.super = ModPolicy.REQUIRED
			if signal.req_mods & GLFW_MOD_CAPS_LOCK.value:
				d.mods.caps_lock = ModPolicy.REQUIRED
			if signal.req_mods & GLFW_MOD_NUM_LOCK.value:
				d.mods.num_lock = ModPolicy.REQUIRED

		if signal.ban_mods is not None:
			if signal.ban_mods & GLFW_MOD_SHIFT.value:
				d.mods.shift = ModPolicy.FORBIDDEN
			if signal.ban_mods & GLFW_MOD_CONTROL.value:
				d.mods.ctrl = ModPolicy.FORBIDDEN
			if signal.ban_mods & GLFW_MOD_ALT.value:
				d.mods.alt = ModPolicy.FORBIDDEN
			if signal.ban_mods & GLFW_MOD_SUPER.value:
				d.mods.super = ModPolicy.FORBIDDEN
			if signal.ban_mods & GLFW_MOD_CAPS_LOCK.value:
				d.mods.caps_lock = ModPolicy.FORBIDDEN
			if signal.ban_mods & GLFW_MOD_NUM_LOCK.value:
				d.mods.num_lock = ModPolicy.FORBIDDEN

	if signal.modifier is not None:
		if signal.modifier.swizzle is not None:
			d.conversion.swizzle = signal.modifier.swizzle
		if signal.modifier.multiplier is not None:
			d.conversion.multiplier[0] = signal.modifier.multiplier[0] if 0 < len(signal.modifier.multiplier) else 1.0
			d.conversion.multiplier[1] = signal.modifier.multiplier[1] if 1 < len(signal.modifier.multiplier) else 1.0
			d.conversion.multiplier[2] = signal.modifier.multiplier[2] if 2 < len(signal.modifier.multiplier) else 1.0
		if signal.modifier.invert is not None:
			d.conversion.invert[0] = signal.modifier.invert[0] if 0 < len(signal.modifier.invert) else False
			d.conversion.invert[1] = signal.modifier.invert[1] if 1 < len(signal.modifier.invert) else False
			d.conversion.invert[2] = signal.modifier.invert[2] if 2 < len(signal.modifier.invert) else False
		if signal.modifier.conversion is not None:
			match d.basic.type:
				case InputType.KEY | InputType.MOUSE_BUTTON | InputType.GAMEPAD_BUTTON:
					d.conversion.dim = OLY_CONVERSION_0D_NAMES.index(signal.modifier.conversion) + 1
				case InputType.GAMEPAD_1D_AXIS:
					d.conversion.dim = OLY_CONVERSION_1D_NAMES.index(signal.modifier.conversion) + 1
				case InputType.GAMEPAD_2D_AXIS | InputType.CURSOR_POSITION | InputType.SCROLL:
					d.conversion.dim = OLY_CONVERSION_2D_NAMES.index(signal.modifier.conversion) + 1

	return d


@dataclass
class EditorMapping:
	name: str
	signals: list[str] = field(default_factory=lambda: [])


@dataclass
class OlyMapping:
	id: str
	signals: list[str] = field(default_factory=lambda: [])

	@staticmethod
	def from_dict(d: dict) -> OlyMapping:
		return OlyMapping(id=d['id'], signals=d['signals'] if 'signals' in d else [])

	def to_dict(self) -> dict:
		return {
			'id': self.id,
			'signals': self.signals
		}


def convert_mapping_from_editor_to_oly_format(mapping: EditorMapping) -> OlyMapping:
	return OlyMapping(id=mapping.name, signals=mapping.signals)


def convert_mapping_from_oly_to_editor_format(mapping: OlyMapping) -> EditorMapping:
	return EditorMapping(name=mapping.id, signals=mapping.signals)
