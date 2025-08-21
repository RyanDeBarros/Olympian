import posixpath
from pathlib import Path
from typing import Optional

from PySide6.QtGui import QUndoStack

PROJECT_FILE: Optional[str] = None
UNDO_STACK = QUndoStack()


def project_root_folder() -> Path:
	global PROJECT_FILE
	return Path(posixpath.dirname(PROJECT_FILE))


def project_resource_folder() -> Path:
	global PROJECT_FILE
	return project_root_folder().joinpath("res")
