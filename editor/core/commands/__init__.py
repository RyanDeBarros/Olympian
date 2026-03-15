import os
import glob
import importlib

from editor.core.REPL import REPLStateMachine


def register_commands(machine: REPLStateMachine):
	package_dir = os.path.dirname(__file__)

	for file in glob.glob(os.path.join(package_dir, "*.py")):
		module_name = os.path.splitext(os.path.basename(file))[0]
		if module_name == "__init__":
			continue
		module = importlib.import_module(f"{__package__}.{module_name}")
		module.register(machine)
