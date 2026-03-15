import sys
from pathlib import Path

from core import REPL

sys.path.append(Path(__file__).parent.parent.as_posix())

if __name__ == "__main__":
	REPL.run()
