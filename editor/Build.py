import shutil
import subprocess
import sys
from pathlib import Path

GREEN = "\033[92m"
YELLOW = "\033[93m"
RED = "\033[91m"
RESET = "\033[0m"

try:
	import PyInstaller
except ImportError:
	print(f"{RED}ERROR: PyInstaller is not installed. Please intall it with:\n\npip install pyinstaller\n{RESET}")
	exit(1)

SCRIPT = Path(__file__).parent / "OlyEditor.py"
EXE = SCRIPT.stem + (".exe" if sys.platform == "win32" else "")
SOURCE_PATH = SCRIPT.parent / "dist" / EXE
DEST_PATH = SCRIPT.parent / EXE


def run():
	print(f"{YELLOW}Building with pyinstaller...{RESET}")
	subprocess.run(["pyinstaller", "--onefile", "--windowed", SCRIPT], cwd=SCRIPT.parent, check=True)
	print(f"{YELLOW}Executable built in 'dist'. Moving it from 'dist' to '.'...{RESET}")
	shutil.move(SOURCE_PATH, DEST_PATH)
	print(f"{YELLOW}Executable moved from 'dist' to '.'. Removing 'dist'...{RESET}")
	shutil.rmtree(SOURCE_PATH.parent)
	print(f"{YELLOW}'dist' removed.{RESET}")
	print(f"{GREEN}OlyEditor executable successfully built.{RESET}")


if __name__ == "__main__":
	run()
