from typing import override

from editor.core import REPLCommand, ProgramState, Resolver


class VarSetCommand(REPLCommand):
	def __init__(self):
		super().__init__("var.set")

	@override
	def requires_initialized_editor(self):
		return False

	@override
	def execute(self):
		program = ProgramState.instance()
		if len(program.args) > 0 and len(program.args) & 1 == 0:
			for i in range(len(program.args) // 2):
				key = program.args[2 * i]
				value = program.args[2 * i + 1]
				if Resolver.is_valid_macro_key(key):
					program.macros.temporary.set(key, value)
				else:
					self.print_arg_error(f"${key} must only contain alphanumeric characters, '-', or '_'")
		else:
			self.print_arg_error("Expected a non-zero even number of arguments")

	@override
	def help(self):
		print("help not implemented")  # DOC

	@override
	def expand_macros(self):
		return False


def register():
	ProgramState.instance().machine.default().add_command(VarSetCommand())
