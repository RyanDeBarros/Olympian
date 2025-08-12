import posixpath
from typing import Optional

PROJECT_FILE: Optional[str] = None


def project_resource_folder():
	global PROJECT_FILE
	return posixpath.join(posixpath.dirname(PROJECT_FILE), "res")
