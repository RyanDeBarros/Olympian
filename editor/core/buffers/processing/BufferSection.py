class BufferSection:
	def __init__(self, name: str, parent: "BufferSection | None"):
		self.name = name
		self.parent = parent
		if self.parent:
			self.level = self.parent.level + 1
		else:
			self.level = 0
		self.children: list[BufferSection] = []
		self.fields: list[str] = []

	def title(self) -> str:
		return ';' * self.level + f" {self.name}"
