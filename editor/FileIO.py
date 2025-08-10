from pathlib import Path

import send2trash


def move_to_trash(path):
	send2trash.send2trash(Path(path).resolve())
