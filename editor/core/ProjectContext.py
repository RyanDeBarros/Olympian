import posixpath
from typing import Optional

from PySide6.QtGui import QUndoStack

PROJECT_FILE: Optional[str] = None
UNDO_STACK = QUndoStack()


def project_resource_folder():
	global PROJECT_FILE
	return posixpath.join(posixpath.dirname(PROJECT_FILE), "res")
