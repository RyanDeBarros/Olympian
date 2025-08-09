class GLParamList:
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
		elif value is str:
			return value
		else:
			raise KeyError(f"No GL Param with value {value}.")


GL_PARAM_LIST = GLParamList()


class GLParam:
	def __init__(self, value, name):
		self.value = value
		self.name = name
		GL_PARAM_LIST.name_to_value[name] = value
		GL_PARAM_LIST.value_to_name[value] = name


GL_NEAREST = GLParam(0x2600, "Nearest")
GL_LINEAR = GLParam(0x2601, "Linear")
GL_NEAREST_MIPMAP_NEAREST = GLParam(0x2700, "Nearest Mipmap Nearest")
GL_LINEAR_MIPMAP_NEAREST = GLParam(0x2701, "Linear Mipmap Nearest")
GL_NEAREST_MIPMAP_LINEAR = GLParam(0x2702, "Nearest Mipmap Linear")
GL_LINEAR_MIPMAP_LINEAR = GLParam(0x2703, "Linear Mipmap Linear")

GL_CLAMP_TO_EDGE = GLParam(0x812F, "Clamp To Edge")
GL_CLAMP_TO_BORDER = GLParam(0x812D, "Clamp To Border")
GL_MIRRORED_REPEAT = GLParam(0x8370, "Mirrored Repeat")
GL_REPEAT = GLParam(0x2901, "Repeat")
GL_MIRROR_CLAMP_TO_EDGE = GLParam(0x8743, "Mirror Clamp To Edge")

# TODO v3 same type of parameter thing for constants like STORAGE_DISCARD, STORAGE_KEEP, MIPMAP_OFF, MIPMAP_AUTO, MIPMAP_MANUAL, etc.
