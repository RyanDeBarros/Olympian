import os.path
import sys
from pathlib import Path

sys.path.append(str(Path(__file__).parent.parent))

from editor.core import REPLRun
from editor.tools import eprint

if __name__ == "__main__":
	if getattr(sys, "frozen", False):
		arg_index = 1
	else:
		arg_index = 2

	if len(sys.argv) > arg_index:
		project_dir = sys.argv[arg_index]
		if os.path.isdir(project_dir):
			os.chdir(project_dir)
		else:
			eprint(f"{project_dir} is not a valid directory")

	REPLRun.run()
