from enum import Enum
from typing import Type, TypeVar, Mapping, Any, Callable

TRealKey = TypeVar("TRealKey", bound=Enum)
TVirtualKey = TypeVar("TVirtualKey", bound=Enum)
TRealEnum = TypeVar("TRealEnum", bound=Enum)
TVirtualEnum = TypeVar("TVirtualEnum", bound=Enum)
TNumber = TypeVar("TNumber")


class EnumField:
	def __init__(self, real_key: TRealKey, virtual_key: TVirtualKey, real_enum: Type[TRealEnum], virtual_enum: Type[TVirtualEnum], description: str):
		self.real_key = real_key
		self.virtual_key = virtual_key
		self.real_enum = real_enum
		self.virtual_enum = virtual_enum
		default = self.get_default()
		self.options = ', '.join(e.value if e != default else f"{e.value}*" for e in virtual_enum)
		self.description = description

	def get_default(self) -> TVirtualEnum | None:
		default = getattr(self.virtual_enum, "default", None)
		if isinstance(default, Callable):
			return default()
		elif default is not None:
			return default
		else:
			return None

	def get_value(self, data: Mapping[str, Any], default: TVirtualEnum | None = None) -> TVirtualEnum:
		try:
			# noinspection PyTypeChecker
			return self.virtual_enum[self.real_enum(data[self.real_key.value]).name]
		except (KeyError, ValueError):
			if default is not None:
				return default
			else:
				default = self.get_default()
				if default is not None:
					return default
				else:
					raise


class RangedNumberField:
	def __init__(self, real_key: TRealKey, virtual_key: TVirtualKey, min_number: TNumber, max_number: TNumber,
				 include_min: bool, include_max: bool, default: TNumber, description: str):
		self.real_key = real_key
		self.virtual_key = virtual_key
		self.min_number = min_number
		self.max_number = max_number
		self.include_min = include_min
		self.include_max = include_max
		self.default = default
		self.description = description
		self.options = f"{'[' if include_min else '('}{min_number}, {max_number}{']' if include_max else ')'}"

	def get_value(self, data: Mapping[str, Any]) -> TNumber:
		# noinspection PyTypeChecker
		return data.get(self.real_key.value, self.default)


class DiscreteNumberField:
	def __init__(self, real_key: TRealKey, virtual_key: TVirtualKey, options: list[TNumber], default: TNumber, description: str = ""):
		self.real_key = real_key
		self.virtual_key = virtual_key
		self.default = default
		self.options = ', '.join(str(option) if option != default else f"{option}*" for option in options)
		self.description = description

	def get_value(self, data: Mapping[str, Any]) -> TNumber:
		# noinspection PyTypeChecker
		return data.get(self.real_key.value, self.default)


class BoolField:
	def __init__(self, real_key: TRealKey, virtual_key: TVirtualKey, default: bool, description: str = ""):
		self.real_key = real_key
		self.virtual_key = virtual_key
		self.default = default
		self.options = f"{'true' if default != True else 'true*'}, {'false' if default != False else 'false*'}"
		self.description = description

	def get_value(self, data: Mapping[str, Any]) -> bool:
		# noinspection PyTypeChecker
		return data.get(self.real_key.value, self.default)
