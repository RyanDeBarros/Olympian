from __future__ import annotations

from dataclasses import dataclass, fields, field

from editor.core import GL_NEAREST, GL_CLAMP_TO_EDGE, OLY_DISCARD, OLY_OFF, OLY_COMMON


@dataclass
class RasterTextureDefaults:
	storage: str = OLY_DISCARD.value
	generate_mipmaps: bool = False
	min_filter: int = GL_NEAREST.value
	mag_filter: int = GL_NEAREST.value
	wrap_s: int = GL_CLAMP_TO_EDGE.value
	wrap_t: int = GL_CLAMP_TO_EDGE.value

	@staticmethod
	def from_dict(d: dict) -> RasterTextureDefaults:
		init_values = {}
		if d is not None:
			for f in fields(RasterTextureDefaults):
				if f.name in d:
					match f.name:
						case _:
							init_values[f.name] = d[f.name]
		return RasterTextureDefaults(**init_values)

	def to_dict(self) -> dict:
		d = {}
		for key, value in self.__dict__.items():
			if hasattr(value, "to_dict"):
				d[key] = value.to_dict()
			else:
				d[key] = value
		return d


@dataclass
class SVGTextureDefaults:
	image_storage: str = OLY_DISCARD.value
	abstract_storage: str = OLY_DISCARD.value
	generate_mipmaps: str = OLY_OFF.value
	min_filter: int = GL_NEAREST.value
	mag_filter: int = GL_NEAREST.value
	wrap_s: int = GL_CLAMP_TO_EDGE.value
	wrap_t: int = GL_CLAMP_TO_EDGE.value

	@staticmethod
	def from_dict(d: dict) -> SVGTextureDefaults:
		init_values = {}
		if d is not None:
			for f in fields(SVGTextureDefaults):
				if f.name in d:
					match f.name:
						case _:
							init_values[f.name] = d[f.name]
		return SVGTextureDefaults(**init_values)

	def to_dict(self) -> dict:
		d = {}
		for key, value in self.__dict__.items():
			if hasattr(value, "to_dict"):
				d[key] = value.to_dict()
			else:
				d[key] = value
		return d


@dataclass
class TextureDefaults:
	raster: RasterTextureDefaults = field(default_factory=lambda: RasterTextureDefaults())
	svg: SVGTextureDefaults = field(default_factory=lambda: SVGTextureDefaults())

	@staticmethod
	def from_dict(d: dict) -> TextureDefaults:
		init_values = {}
		if d is not None:
			for f in fields(TextureDefaults):
				if f.name in d:
					match f.name:
						case 'raster':
							init_values[f.name] = RasterTextureDefaults.from_dict(d[f.name])
						case 'svg':
							init_values[f.name] = SVGTextureDefaults.from_dict(d[f.name])
						case _:
							init_values[f.name] = d[f.name]
		return TextureDefaults(**init_values)

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
	def from_dict(d: dict) -> FontFaceDefaults:
		init_values = {}
		if d is not None:
			for f in fields(FontFaceDefaults):
				if f.name in d:
					match f.name:
						case _:
							init_values[f.name] = d[f.name]
		return FontFaceDefaults(**init_values)

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
	def from_dict(d: dict) -> FontAtlasDefaults:
		init_values = {}
		if d is not None:
			for f in fields(FontAtlasDefaults):
				if f.name in d:
					match f.name:
						case _:
							init_values[f.name] = d[f.name]
		return FontAtlasDefaults(**init_values)

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
	def from_dict(d: dict) -> FontDefaults:
		init_values = {}
		if d is not None:
			for f in fields(FontDefaults):
				if f.name in d:
					match f.name:
						case 'font_face':
							init_values[f.name] = FontFaceDefaults.from_dict(d[f.name])
						case 'font_atlas':
							init_values[f.name] = FontAtlasDefaults.from_dict(d[f.name])
						case _:
							init_values[f.name] = d[f.name]
		return FontDefaults(**init_values)

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
