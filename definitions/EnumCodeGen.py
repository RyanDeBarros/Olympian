import re
import sys
from pathlib import Path

ENGINE_DIR = Path("engine")
DEFINITIONS_DIR = Path("definitions")
ENUMS_DIR = (DEFINITIONS_DIR / "enums")
HEADER_ROOT_DIR = (ENGINE_DIR / "gen")
CPP_ROOT_DIR = (ENGINE_DIR / ".gen")
COL_COUNT = 3


# TODO v7 cache enum file timestamps

class EnumRow:
	def __init__(self):
		self.name: str = ""
		self.gl_enum: int = -1
		self.comment: str = ""


def build(enum_file: Path, rows: list[EnumRow]) -> None:
	rel_path = enum_file.relative_to(DEFINITIONS_DIR)
	header_path = HEADER_ROOT_DIR / rel_path.with_suffix(".h")
	cpp_path = CPP_ROOT_DIR / rel_path.with_suffix(".cpp")
	cpp_path.parent.mkdir(parents=True, exist_ok=True)
	cpp_path.touch(exist_ok=True)

	namespace = f"oly::internal::gen"
	subnamespace = "::".join(rel_path.parts[1:-1])
	if len(subnamespace) > 0:
		namespace += f"::{subnamespace}"

	fn_name = re.sub(r'([a-z0-9])([A-Z])', r'\1_\2', rel_path.stem).lower()

	enum_def = ""
	for i, row in enumerate(rows):
		enum_def += f"\t\t{row.gl_enum}"
		if i + 1 < len(rows):
			enum_def += ","
		if len(row.comment) > 0:
			if i + 1 == len(rows):
				enum_def += " "
			enum_def += f" // {row.comment}"
		if i + 1 < len(rows):
			enum_def += '\n'

	codegen = f"""#include \"{header_path.relative_to(ENGINE_DIR).as_posix()}\"

#include <array>

namespace {namespace}
{{
	constexpr unsigned int LIMIT = {len(rows)};
	const std::array<GLenum, LIMIT> DEFS = {{
{enum_def}
	}};

	GLenum {fn_name}(unsigned int index)
	{{
		return DEFS.at(index);
	}}
}}
"""

	cpp_path.write_text(codegen)
	print(f"Wrote to {cpp_path.resolve().as_posix()}")


def gen(enum_file: Path) -> list[str]:
	lines = enum_file.read_text().splitlines()
	names: dict[str, int] = {}
	indexes: dict[int, int] = {}
	gl_enums: dict[int, int] = {}
	comments: dict[int, str] = {}

	errors = []

	for i, line in enumerate(lines):
		i += 1  # offset for line number
		line, comment = line.split('#', maxsplit=1)
		line = line.strip()
		if not line:
			continue

		name_match = re.match(r'\[(.*?)]', line)
		if not name_match:
			errors.append(f"Line {i}: missing or malformed name in []")
			continue

		name = name_match.group(1).strip()
		if name not in names:
			names[name] = i
		else:
			errors.append(f"Line {i}: {name} is duplicate name (first appeared at line {names[name]})")

		rest = line[name_match.end():].strip().split()
		if 1 + len(rest) != COL_COUNT:
			errors.append(f"Line {i}: {1 + len(rest)} columns (expected {COL_COUNT})")
			continue

		try:
			index = int(rest[0])
			if index not in indexes:
				indexes[index] = i
			else:
				errors.append(f"Line {i}: {index} is duplicate index (first appeared at line {indexes[index]})")
		except ValueError:
			errors.append(f"Line {i}: {rest[0]} is not an integer")

		try:
			gl_enum = int(rest[1], 0)
			if gl_enum not in gl_enums:
				gl_enums[gl_enum] = i
			else:
				errors.append(f"Line {i}: {gl_enum} is duplicate GL enum (first appeared at line {gl_enums[gl_enum]})")
		except ValueError:
			errors.append(f"Line {i}: {rest[1]} is not an integer")

		comments[i] = comment.strip()

	row_len = len(lines)
	if row_len == 0:
		errors.append(f"No rows!")

	if any(index not in indexes for index in range(row_len)):
		errors.append(f"Index set is not a range({row_len})")

	if len(errors) > 0:
		return errors

	rows = [EnumRow() for _ in range(row_len)]
	lut = {v: k for k, v in indexes.items()}

	for name, i in names.items():
		rows[lut[i]].name = name

	for gl_enum, i in gl_enums.items():
		rows[lut[i]].gl_enum = gl_enum

	for i, comment in comments.items():
		rows[lut[i]].comment = comment

	build(enum_file, rows)
	return errors


if __name__ == "__main__":
	fail = False
	for enum_file in ENUMS_DIR.rglob("*.enum"):
		errors = gen(enum_file)
		for error in errors:
			print(f"Enum generation failed for {enum_file.relative_to(ENUMS_DIR).as_posix()}: {error}")
			fail = True

	if fail:
		sys.exit(1)
