from editor.tools import eprint


class REPLError(RuntimeError):
	def __init__(self, description: str):
		self.description = description
		super().__init__(description)

	def print(self):
		eprint(self.description)
