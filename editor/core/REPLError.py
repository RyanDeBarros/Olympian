from editor.tools import eprint


class REPLError(RuntimeError):
	def __init__(self, description: str):
		super().__init__(description)
		self.description = description

	def print(self):
		eprint(self.description)
