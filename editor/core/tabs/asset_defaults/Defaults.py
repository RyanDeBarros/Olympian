from __future__ import annotations

from dataclasses import dataclass, fields, field

from ..asset_structures import Texture, Font

@dataclass
class Defaults:
	texture: Texture = field(default_factory=lambda: Texture())
	font: Font = field(default_factory=lambda: Font())

	@staticmethod
	def from_dict(d: dict) -> Defaults:
		init_values = {}
		if d is not None:
			for f in fields(Defaults):
				if f.name in d:
					match f.name:
						case 'texture':
							init_values[f.name] = Texture.from_dict(d[f.name])
						case 'font':
							init_values[f.name] = Font.from_dict(d[f.name])
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
