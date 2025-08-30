class EditorPreferences:
	def __init__(self):
		self.prompt_user_when_deleting_paths = True
		self.texture_file_extensions = ['.png', '.jpg', '.jpeg', '.bmp', '.gif', '.svg']
		self.font_file_extensions = ['.ttf', '.otf']


PREFERENCES = EditorPreferences()
