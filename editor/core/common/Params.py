class _ParamList:
	def __init__(self):
		self.name_to_value = {}
		self.value_to_name = {}

	def get_value(self, name):
		if name in self.name_to_value:
			return self.name_to_value[name]
		else:
			raise KeyError(f"No GL Param with name {name}.")

	def get_name(self, value):
		if value in self.value_to_name:
			return self.value_to_name[value]
		elif type(value) is str:
			return value.title()
		else:
			raise KeyError(f"No GL Param with value {value}.")


PARAM_LIST = _ParamList()


class Param:
	def __init__(self, value, name):
		self.value = value
		self.name = name
		PARAM_LIST.name_to_value[name] = value
		PARAM_LIST.value_to_name[value] = name


GL_NEAREST = Param(0x2600, "Nearest")
GL_LINEAR = Param(0x2601, "Linear")
GL_NEAREST_MIPMAP_NEAREST = Param(0x2700, "Nearest Mipmap Nearest")
GL_LINEAR_MIPMAP_NEAREST = Param(0x2701, "Linear Mipmap Nearest")
GL_NEAREST_MIPMAP_LINEAR = Param(0x2702, "Nearest Mipmap Linear")
GL_LINEAR_MIPMAP_LINEAR = Param(0x2703, "Linear Mipmap Linear")

GL_CLAMP_TO_EDGE = Param(0x812F, "Clamp To Edge")
GL_CLAMP_TO_BORDER = Param(0x812D, "Clamp To Border")
GL_MIRRORED_REPEAT = Param(0x8370, "Mirrored Repeat")
GL_REPEAT = Param(0x2901, "Repeat")
GL_MIRROR_CLAMP_TO_EDGE = Param(0x8743, "Mirror Clamp To Edge")

GLFW_MOD_SHIFT = Param(0x0001, 'Mod Shift')
GLFW_MOD_CONTROL = Param(0x0002, 'Mod Ctrl')
GLFW_MOD_ALT = Param(0x0004, 'Mod Alt')
GLFW_MOD_SUPER = Param(0x0008, 'Mod Super')
GLFW_MOD_CAPS_LOCK = Param(0x0010, 'Mod Caps Lock')
GLFW_MOD_NUM_LOCK = Param(0x0020, 'Mod Num Lock')

OLY_DISCARD = Param("discard", "Discard")
OLY_KEEP = Param("keep", "Keep")

OLY_OFF = Param("off", "Off")
OLY_AUTO = Param("auto", "Auto")
OLY_MANUAL = Param("manual", "Manual")

OLY_COMMON = Param("common", "Common")
OLY_ALPHA_NUMERIC = Param("alpha_numeric", "Alpha Numeric")
OLY_NUMERIC = Param("numeric", "Numeric")
OLY_ALPHABET = Param("alphabet", "Alphabet")
OLY_ALPHABET_LOWERCASE = Param("alphabet_lowercase", "Alphabet (lowercase)")
OLY_ALPHABET_UPPERCASE = Param("alphabet_uppercase", "Alphabet (uppercase)")
