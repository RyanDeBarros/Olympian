from enum import Enum
from typing import Type, TypeVar, Mapping, Any, Callable

TRealKey = TypeVar("TRealKey", bound=Enum)
TVirtualKey = TypeVar("TVirtualKey", bound=Enum)
TRealEnum = TypeVar("TRealEnum", bound=Enum)
TVirtualEnum = TypeVar("TVirtualEnum", bound=Enum)


def get_default(virtual_enum: Type[TVirtualEnum]) -> TVirtualEnum | None:
	default = getattr(virtual_enum, "default", None)
	if isinstance(default, Callable):
		return default()
	elif default is not None:
		return default
	else:
		return None


def get_enum(data: Mapping[str, Any], key: TRealKey, real_enum: Type[TRealEnum], virtual_enum: Type[TVirtualEnum], default: TVirtualEnum | None = None) -> TVirtualEnum:
	try:
		return virtual_enum[real_enum(data[key.value]).name]
	except (KeyError, ValueError):
		if default is not None:
			return default
		else:
			default = get_default(virtual_enum)
			if default is not None:
				return default
			else:
				raise
