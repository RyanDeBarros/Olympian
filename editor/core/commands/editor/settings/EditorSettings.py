from pathlib import Path


# TODO v7 inherit from AbstractBuffer
class EditorSettings:
	def __init__(self, persistent_path: Path):
		self.persistent_path = persistent_path

	def load(self):
		pass  # TODO v7.2
