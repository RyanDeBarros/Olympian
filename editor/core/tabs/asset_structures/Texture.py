from __future__ import annotations

from dataclasses import dataclass, fields, field

from editor.core import GL_NEAREST, GL_CLAMP_TO_EDGE, OLY_DISCARD, OLY_OFF

@dataclass
class RasterTexture:
	storage: str = OLY_DISCARD.value
	generate_mipmaps: bool = False
	min_filter: int = GL_NEAREST.value
	mag_filter: int = GL_NEAREST.value
	wrap_s: int = GL_CLAMP_TO_EDGE.value
	wrap_t: int = GL_CLAMP_TO_EDGE.value

	@staticmethod
	def from_dict(d: dict) -> RasterTexture:
		init_values = {}
		if d is not None:
			for f in fields(RasterTexture):
				if f.name in d:
					match f.name:
						case _:
							init_values[f.name] = d[f.name]
		return RasterTexture(**init_values)

	def to_dict(self) -> dict:
		d = {}
		for key, value in self.__dict__.items():
			if hasattr(value, "to_dict"):
				d[key] = value.to_dict()
			else:
				d[key] = value
		return d


@dataclass
class SVGTexture:
	image_storage: str = OLY_DISCARD.value
	abstract_storage: str = OLY_DISCARD.value
	generate_mipmaps: str = OLY_OFF.value
	min_filter: int = GL_NEAREST.value
	mag_filter: int = GL_NEAREST.value
	wrap_s: int = GL_CLAMP_TO_EDGE.value
	wrap_t: int = GL_CLAMP_TO_EDGE.value

	@staticmethod
	def from_dict(d: dict) -> SVGTexture:
		init_values = {}
		if d is not None:
			for f in fields(SVGTexture):
				if f.name in d:
					match f.name:
						case _:
							init_values[f.name] = d[f.name]
		return SVGTexture(**init_values)

	def to_dict(self) -> dict:
		d = {}
		for key, value in self.__dict__.items():
			if hasattr(value, "to_dict"):
				d[key] = value.to_dict()
			else:
				d[key] = value
		return d


@dataclass
class Texture:
	raster: RasterTexture = field(default_factory=lambda: RasterTexture())
	svg: SVGTexture = field(default_factory=lambda: SVGTexture())

	@staticmethod
	def from_dict(d: dict) -> Texture:
		init_values = {}
		if d is not None:
			for f in fields(Texture):
				if f.name in d:
					match f.name:
						case 'raster':
							init_values[f.name] = RasterTexture.from_dict(d[f.name])
						case 'svg':
							init_values[f.name] = SVGTexture.from_dict(d[f.name])
						case _:
							init_values[f.name] = d[f.name]
		return Texture(**init_values)

	def to_dict(self) -> dict:
		d = {}
		for key, value in self.__dict__.items():
			if hasattr(value, "to_dict"):
				d[key] = value.to_dict()
			else:
				d[key] = value
		return d
