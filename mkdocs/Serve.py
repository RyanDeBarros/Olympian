import os
import subprocess
import sys
import webbrowser
from time import sleep


def serve():
	if sys.platform == "win32":
		subprocess.Popen(["cmd", "/c", "start", "cmd", "/k", "mkdocs serve"])
	elif sys.platform == "darwin":
		subprocess.Popen(["osascript", "-e", f'tell app "Terminal" to do script "mkdocs serve"'])
	else:
		subprocess.Popen(["x-terminal-emulator", "-e", f"bash -c 'mkdocs serve'"])

	sleep(3.0)
	webbrowser.open("http://127.0.0.1:8000")


if __name__ == "__main__":
	serve()
