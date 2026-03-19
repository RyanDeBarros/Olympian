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


# TODO v7 buffers:
#    * data: for asset I/O
#    * editor settings
#    * macro (temp) var I/O
#    * large output dump: add options to certain commands like list to print in that buffer instead of cmdline
#    * error: instead of in cmdline, print asset syntax/format errors in buffer. Just print in cmdline that the error buffer should be checked
#    Multiple data/error buffers so multiple assets can be open at the same time?
#    buffers.<cmd> <buffer-name>: custom autocomplete for buffer names in argument
#    buffers.open: open with default app (likely IDE/VSCode). To make this work consistently, use custom file extension for buffers (.olybuf?)
#    buffers.reveal: open buffer in file explorer. The path should be in AppData, under some subfolder that corresponds to a manifest mapping of the project, to allow for multiple buffers for different projects.
#    buffers.path: print path of buffer file
