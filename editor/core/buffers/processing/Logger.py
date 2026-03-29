from editor.tools import eprint


# TODO v7.2 use log file or new terminal instance for output
class Logger:
	@staticmethod
	def log_info(*values) -> None:
		print(*values)

	@staticmethod
	def log_error(*values) -> None:
		eprint(*values)
