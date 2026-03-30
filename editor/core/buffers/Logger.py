import platform
import subprocess
from pathlib import Path
import tempfile
import atexit


# TODO v7 editor setting to redirect Logger output to a logfile in .editor/log instead
class Logger:
	CWD: Path
	LOG_FILE: Path | None = None
	TERM: subprocess.Popen | None = None

	@staticmethod
	def init():
		atexit.register(Logger.cleanup)

	@staticmethod
	def ensure_log_exists():
		if Logger.LOG_FILE is None or not Logger.LOG_FILE.exists():
			temp = tempfile.NamedTemporaryFile(prefix="oly_editor_")
			Logger.LOG_FILE = Path(temp.name)
			temp.close()

	@staticmethod
	def ensure_terminal_open():
		if Logger.TERM is None or Logger.TERM.poll() is not None:
			if platform.system() == "Windows":
				Logger.TERM = subprocess.Popen(["python", "OutputTerminal.py", str(Logger.LOG_FILE)], creationflags=subprocess.CREATE_NEW_CONSOLE, cwd=Logger.CWD)
			elif platform.system() == "Darwin":
				script = f'tell application "Terminal" to do script "python3 OutputTerminal.py \\"{Logger.LOG_FILE}\\"; clear"'
				Logger.TERM = subprocess.Popen(["osascript", "-e", script], cwd=Logger.CWD)
			else:
				Logger.TERM = subprocess.Popen(["xterm", "-e", f'python3 -u OutputTerminal.py "{Logger.LOG_FILE}"'], cwd=Logger.CWD)

	@staticmethod
	def ensure_loggable():
		Logger.ensure_log_exists()
		Logger.ensure_terminal_open()

	@staticmethod
	def cleanup():
		if Logger.TERM.poll() is None:
			Logger.TERM.terminate()

		Logger.LOG_FILE.unlink()

	@staticmethod
	def log_info(*values, sep=' '):
		Logger.ensure_loggable()
		with Logger.LOG_FILE.open("a", encoding="utf-8") as f:
			f.write(sep.join(str(v) for v in values) + "\n")

	@staticmethod
	def log_error(*values, sep=' '):
		Logger.ensure_loggable()
		with Logger.LOG_FILE.open("a", encoding="utf-8") as f:
			f.write("ERROR: " + sep.join(str(v) for v in values) + "\n")
