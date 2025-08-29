from __future__ import annotations

from dataclasses import dataclass, fields, field

from editor.core import GL_NEAREST, OLY_DISCARD, OLY_COMMON


@dataclass
class FontFace:
	storage: str = OLY_DISCARD.value

	@staticmethod
	def from_dict(d: dict) -> FontFace:
		init_values = {}
		if d is not None:
			for f in fields(FontFace):
				if f.name in d:
					match f.name:
						case _:
							init_values[f.name] = d[f.name]
		return FontFace(**init_values)

	def to_dict(self) -> dict:
		d = {}
		for key, value in self.__dict__.items():
			if hasattr(value, "to_dict"):
				d[key] = value.to_dict()
			else:
				d[key] = value
		return d

@dataclass
class FontAtlas:
	storage: str = OLY_DISCARD.value
	font_size: int = 32
	min_filter: int = GL_NEAREST.value
	mag_filter: int = GL_NEAREST.value
	generate_mipmaps: bool = False
	common_buffer_preset: str = OLY_COMMON.value
	common_buffer: str = ""
	use_common_buffer_preset: bool = True

	@staticmethod
	def from_dict(d: dict) -> FontAtlas:
		init_values = {}
		if d is not None:
			for f in fields(FontAtlas):
				if f.name in d:
					match f.name:
						case _:
							init_values[f.name] = d[f.name]
		return FontAtlas(**init_values)

	def to_dict(self) -> dict:
		d = {}
		for key, value in self.__dict__.items():
			if hasattr(value, "to_dict"):
				d[key] = value.to_dict()
			else:
				d[key] = value
		return d

@dataclass
class Font:
	font_face: FontFace = field(default_factory=lambda: FontFace())
	font_atlas: FontAtlas = field(default_factory=lambda: FontAtlas())

	@staticmethod
	def from_dict(d: dict) -> Font:
		init_values = {}
		if d is not None:
			for f in fields(Font):
				if f.name in d:
					match f.name:
						case 'font_face':
							init_values[f.name] = FontFace.from_dict(d[f.name])
						case 'font_atlas':
							init_values[f.name] = FontAtlas.from_dict(d[f.name])
						case _:
							init_values[f.name] = d[f.name]
		return Font(**init_values)

	def to_dict(self) -> dict:
		d = {}
		for key, value in self.__dict__.items():
			if hasattr(value, "to_dict"):
				d[key] = value.to_dict()
			else:
				d[key] = value
		return d


@dataclass
class Kerning:
	pair: list[str] = field(default_factory=lambda: ["", ""])
	dist: int = 0

	@staticmethod
	def from_dict(d: dict) -> Kerning:
		init_values = {}
		if d is not None:
			for f in fields(Kerning):
				if f.name in d:
					match f.name:
						case _:
							init_values[f.name] = d[f.name]
		return Kerning(**init_values)

	def to_dict(self) -> dict:
		d = {}
		for key, value in self.__dict__.items():
			if hasattr(value, "to_dict"):
				d[key] = value.to_dict()
			else:
				d[key] = value
		return d
