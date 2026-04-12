from pathlib import Path

import toml
from toml import TomlDecodeError

from . import CodeGen

ENGINE_DIR = Path("engine")
DEFINITIONS_DIR = Path("definitions")
ENUMS_DIR = DEFINITIONS_DIR / "enums"
GEN_ROOT_DIR = ENGINE_DIR / ".gen"
GEN_ENUMS_DIR = GEN_ROOT_DIR / "enums"


class EnumEntry:
	def __init__(self, entry: dict):
		self.label = str(entry['label'])
		self.index = int(entry['index'])
		self.enum = int(entry['enum'])
		self.comment = str(entry.get('comment', ''))
		self.tooltip = str(entry.get('tooltip', ''))


def build(enum_file: Path, rows: list[EnumEntry], index_type: str, enum_type: str, default_enum: int) -> None:
	rel_path = enum_file.relative_to(DEFINITIONS_DIR)
	inl_path = GEN_ROOT_DIR / rel_path.with_suffix(".inl")
	inl_path.parent.mkdir(parents=True, exist_ok=True)
	inl_path.touch(exist_ok=True)

	namespace = f"oly::_gen"
	subnamespace = "::".join(rel_path.parts[1:-1])
	if len(subnamespace) > 0:
		namespace += f"::{subnamespace}"

	enum_def = ""
	for i, row in enumerate(rows):
		enum_def += f"\t\t\tstatic_cast<{enum_type}>({row.enum})"
		if i + 1 < len(rows):
			enum_def += ","
		if len(row.comment) > 0:
			if i + 1 == len(rows):
				enum_def += " "
			enum_def += f"  // {row.comment}"
		if i + 1 < len(rows):
			enum_def += '\n'

	codegen = f"""#include <array>
#include <optional>

namespace {namespace}
{{
	class {enum_file.stem}
	{{
		static inline const std::array<{enum_type}, {len(rows)}> DEFINITIONS = {{
{enum_def}
		}};
		
		{enum_file.stem}() = delete;
		~{enum_file.stem}() = delete;

	public:
		static {enum_type} val({index_type} index)
		{{
			return DEFINITIONS.at(index);
		}}

		static {enum_type} val()
		{{
			return static_cast<{enum_type}>({default_enum});
		}}

		static {enum_type} val(std::optional<{index_type}> index, {enum_type} def = static_cast<{enum_type}>({default_enum}))
		{{
			if (index)
				return DEFINITIONS.at(*index);
			else
				return def;
		}}
	}};
}}
"""

	inl_path.write_text(codegen)


def gen(enum_file: Path, *args, **kwargs) -> list[str]:
	try:
		d = toml.loads(enum_file.read_text())
	except TomlDecodeError as e:
		return [str(e)]

	try:
		index_type = str(d['index_type'])
		enum_type = str(d['enum_type'])
		default = int(d['default'])
	except (KeyError, ValueError) as e:
		return [repr(e)]

	bad_lines: dict[int, str] = {}
	entries: list[EnumEntry] = []
	indexes: dict[int, int] = {}
	enums: dict[int, int] = {}
	try:
		for i, entry in enumerate(d['enum']):
			try:
				e = EnumEntry(entry)
				if e.index in indexes:
					bad_lines[i] = f"duplicate index {e.index} first appeared in enum #{indexes[e.index]}"
				elif e.enum in enums:
					bad_lines[i] = f"duplicate enum value {e.enum} first appeared in enum #{indexes[e.enum]}"
				else:
					indexes[e.index] = i
					enums[e.enum] = i
					entries.append(e)
			except (KeyError, ValueError) as e:
				bad_lines[i] = repr(e)
	except (KeyError, ValueError) as e:
		return [repr(e)]

	if len(bad_lines) > 0:
		return [f"Key #{i + 1}: {error}" for i, error in bad_lines.items()]

	errors = []

	row_len = len(entries)
	if row_len == 0:
		errors.append(f"No enums!")

	if any(index not in indexes for index in range(row_len)):
		errors.append(f"Index set is not a range({row_len})")

	if default not in indexes:
		errors.append(f"Default index {default} is not registered")

	if len(errors) > 0:
		return errors

	build(enum_file, entries, index_type, enum_type, next(entry.enum for entry in entries if entry.index == default))
	return errors


if __name__ == "__main__":
	codegen = CodeGen("enums")
	codegen.process(gen, "*.toml")
