class IntReference:
	def __init__(self, value: int):
		if not isinstance(value, int):
			raise ValueError(f"Value {value} must be an integer")
		self.value = value

	def __repr__(self) -> str:
		return f"IntReference({self.value})"
