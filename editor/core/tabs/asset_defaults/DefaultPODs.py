from __future__ import annotations

from dataclasses import dataclass, fields, field

from editor.core import GL_NEAREST, GL_CLAMP_TO_EDGE, OLY_DISCARD, OLY_OFF, OLY_COMMON


@dataclass
class TextureDefaults:
	raster_storage: str = OLY_DISCARD.value  # regular storage
	svg_storage: str = OLY_DISCARD.value  # svg image_storage
	abstract_storage: str = OLY_DISCARD.value  # svg abstract_storage
	raster_generate_mipmaps: bool = False
	svg_generate_mipmaps: str = OLY_OFF.value
	min_filter: int = GL_NEAREST.value
	mag_filter: int = GL_NEAREST.value
	wrap_s: int = GL_CLAMP_TO_EDGE.value
	wrap_t: int = GL_CLAMP_TO_EDGE.value

	@staticmethod
	def from_dict(d: dict) -> Defaults:
		init_values = {}
		if d is not None:
			for f in fields(Defaults):
				if f.name in d:
					match f.name:
						case _:
							init_values[f.name] = d[f.name]
		return Defaults(**init_values)

	def to_dict(self) -> dict:
		d = {}
		for key, value in self.__dict__.items():
			if hasattr(value, "to_dict"):
				d[key] = value.to_dict()
			else:
				d[key] = value
		return d

@dataclass
class FontFaceDefaults:
	storage: str = OLY_DISCARD.value

	@staticmethod
	def from_dict(d: dict) -> Defaults:
		init_values = {}
		if d is not None:
			for f in fields(Defaults):
				if f.name in d:
					match f.name:
						case _:
							init_values[f.name] = d[f.name]
		return Defaults(**init_values)

	def to_dict(self) -> dict:
		d = {}
		for key, value in self.__dict__.items():
			if hasattr(value, "to_dict"):
				d[key] = value.to_dict()
			else:
				d[key] = value
		return d

@dataclass
class FontAtlasDefaults:
	storage: str = OLY_DISCARD.value
	font_size: int = 32
	min_filter: int = GL_NEAREST.value
	mag_filter: int = GL_NEAREST.value
	generate_mipmaps: bool = False
	common_buffer_preset: str = OLY_COMMON.value
	common_buffer: str = ""

	@staticmethod
	def from_dict(d: dict) -> Defaults:
		init_values = {}
		if d is not None:
			for f in fields(Defaults):
				if f.name in d:
					match f.name:
						case _:
							init_values[f.name] = d[f.name]
		return Defaults(**init_values)

	def to_dict(self) -> dict:
		d = {}
		for key, value in self.__dict__.items():
			if hasattr(value, "to_dict"):
				d[key] = value.to_dict()
			else:
				d[key] = value
		return d

@dataclass
class FontDefaults:
	font_face: FontFaceDefaults = field(default_factory=lambda: FontFaceDefaults())
	font_atlas: FontAtlasDefaults = field(default_factory=lambda: FontAtlasDefaults())

	@staticmethod
	def from_dict(d: dict) -> Defaults:
		init_values = {}
		if d is not None:
			for f in fields(Defaults):
				if f.name in d:
					match f.name:
						case 'font_face':
							init_values[f.name] = FontFaceDefaults.from_dict(d[f.name])
						case 'font_atlas':
							init_values[f.name] = FontAtlasDefaults.from_dict(d[f.name])
						case _:
							init_values[f.name] = d[f.name]
		return Defaults(**init_values)

	def to_dict(self) -> dict:
		d = {}
		for key, value in self.__dict__.items():
			if hasattr(value, "to_dict"):
				d[key] = value.to_dict()
			else:
				d[key] = value
		return d

@dataclass
class Defaults:
	texture: TextureDefaults = field(default_factory=lambda: TextureDefaults())
	font: FontDefaults = field(default_factory=lambda: FontDefaults())

	@staticmethod
	def from_dict(d: dict) -> Defaults:
		init_values = {}
		if d is not None:
			for f in fields(Defaults):
				if f.name in d:
					match f.name:
						case 'texture':
							init_values[f.name] = TextureDefaults.from_dict(d[f.name])
						case 'font':
							init_values[f.name] = FontDefaults.from_dict(d[f.name])
						case _:
							init_values[f.name] = d[f.name]
		return Defaults(**init_values)

	def to_dict(self) -> dict:
		d = {}
		for key, value in self.__dict__.items():
			if hasattr(value, "to_dict"):
				d[key] = value.to_dict()
			else:
				d[key] = value
		return d
