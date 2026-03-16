import os
import importlib

from editor.core.REPL import ProgramState


def register(program: ProgramState):
	package_dir = os.path.dirname(__file__)

	for root, dirs, files in os.walk(package_dir):
		for file in files:
			if not file.endswith(".py") or file == "__init__.py":
				continue

			relative_path = os.path.relpath(str(os.path.join(root, file)), package_dir)
			module_parts = relative_path.replace(os.path.sep, '.').rsplit(".py", 1)[0]
			module = importlib.import_module(f"{__package__}.{module_parts}")
			if hasattr(module, "register"):
				module.register(program)
