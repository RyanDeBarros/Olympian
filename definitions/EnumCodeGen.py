import json
import re
import sys
from pathlib import Path

ENGINE_DIR = Path("engine")
DEFINITIONS_DIR = Path("definitions")
ENUMS_DIR = DEFINITIONS_DIR / "enums"
GEN_ROOT_DIR = ENGINE_DIR / ".gen"
CACHE_FILE = GEN_ROOT_DIR / "cache.json"
COL_COUNT = 3

def load_cache() -> dict:
	cache = CACHE_FILE.read_text() if CACHE_FILE else ""
	return json.loads(cache) if len(cache) > 0 else {}

def save_cache(cache: dict) -> None:
	CACHE_FILE.parent.mkdir(parents=True, exist_ok=True)
	CACHE_FILE.touch(exist_ok=True)
	CACHE_FILE.write_text(json.dumps(cache))

def prune_cache(files_seen: set[str]):
	for inl_file in GEN_ROOT_DIR.rglob("*.inl"):
		cache_entry = inl_file.relative_to(GEN_ROOT_DIR).with_suffix("").as_posix()
		if cache_entry not in files_seen:
			inl_file.unlink()

class EnumRow:
	def __init__(self):
		self.name: str = ""
		self.gl_enum: int = -1
		self.comment: str = ""


def build(enum_file: Path, rows: list[EnumRow], default_gl_enum: int) -> None:
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
		enum_def += f"\t\t\t{row.gl_enum}"
		if i + 1 < len(rows):
			enum_def += ","
		if len(row.comment) > 0:
			if i + 1 == len(rows):
				enum_def += " "
			enum_def += f"  // {row.comment}"
		if i + 1 < len(rows):
			enum_def += '\n'

	codegen = f"""#include \"external/GL.h\"

#include <array>
#include <optional>

namespace {namespace}
{{
	class {enum_file.stem}
	{{
		static inline const std::array<GLenum, {len(rows)}> DEFINITIONS = {{
{enum_def}
		}};
		
		{enum_file.stem}() = delete;
		~{enum_file.stem}() = delete;

	public:
		static GLenum val(unsigned int index)
		{{
			return DEFINITIONS.at(index);
		}}

		static GLenum val()
		{{
			return {default_gl_enum};
		}}

		static GLenum val(std::optional<unsigned int> index)
		{{
			if (index)
				return DEFINITIONS.at(*index);
			else
				return {default_gl_enum};
		}}
	}};
}}
"""

	inl_path.write_text(codegen)


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

	build(enum_file, rows, rows[lut[1]].gl_enum)  # 1-indexed
	return errors


if __name__ == "__main__":
	fail = False
	cache = load_cache()
	new_cache = {}
	files_seen: set[str] = set()

	for enum_file in ENUMS_DIR.rglob("*.enum"):
		cache_entry = enum_file.relative_to(DEFINITIONS_DIR).with_suffix("").as_posix()
		files_seen.add(cache_entry)

		mtime = enum_file.stat().st_mtime

		if cache_entry in cache and cache[cache_entry] == mtime:
			new_cache[cache_entry] = mtime
			continue

		errors = gen(enum_file)
		if errors:
			for error in errors:
				print(f"Enum generation failed for {enum_file.relative_to(ENUMS_DIR).as_posix()}: {error}")
			fail = True
		else:
			new_cache[cache_entry] = mtime

	prune_cache(files_seen)
	save_cache(new_cache)

	if fail:
		sys.exit(1)
