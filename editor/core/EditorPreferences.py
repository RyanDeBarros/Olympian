class EditorPreferences:
	def __init__(self):
		# TODO v3 implement in editor and save/load preferences file in editor, not project
		self.prompt_user_when_deleting_paths = True
		self.undo_stack_limit = 0


PREFERENCES = EditorPreferences()
