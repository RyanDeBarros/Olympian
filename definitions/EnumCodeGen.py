import json
import re
import sys
from pathlib import Path

# TODO v7 replace all ".value<std::string>("/".value_or<std::string>(" instances with codegen enums
# TODO v7 after pruning remove empty folders

ENGINE_DIR = Path("engine")
DEFINITIONS_DIR = Path("definitions")
ENUMS_DIR = DEFINITIONS_DIR / "enums"
GEN_ROOT_DIR = ENGINE_DIR / ".gen"
CACHE_FILE = GEN_ROOT_DIR / "cache.json"
COL_COUNT = 3


def load_cache() -> dict:
	cache = CACHE_FILE.read_text() if CACHE_FILE.exists() else ""
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
		self.enum: int = -1
		self.comment: str = ""


def build(enum_file: Path, rows: list[EnumRow], index_type: str, enum_type: str, default_enum: int) -> None:
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


def gen(enum_file: Path) -> list[str]:
	lines = enum_file.read_text().splitlines()
	index_type: str = ""
	enum_type: str = ""
	names: dict[str, int] = {}
	indexes: dict[int, int] = {}
	enums: dict[int, int] = {}
	comments: dict[int, str] = {}

	errors = []

	for i, line in enumerate(lines):
		if not line or line.isspace():
			continue

		line = line.strip()
		if line.startswith(";"):
			m = re.match(r"\"(?P<index_type>.*?)\"\s*->\s*\"(?P<enum_type>.*?)\"", line[1:].strip())
			if m:
				index_type = m.group("index_type")
				enum_type = m.group("enum_type")
			else:
				errors.append(f"Line {i + 1}: malformed ;-metadata line")

			continue

		line, comment = line.split('#', maxsplit=1)
		line = line.strip()
		if not line:
			continue

		name_match = re.match(r'\[(.*?)]', line)
		if not name_match:
			errors.append(f"Line {i + 1}: missing or malformed name in []")
			continue

		name = name_match.group(1).strip()
		if name not in names:
			names[name] = i
		else:
			errors.append(f"Line {i + 1}: {name} is duplicate name (first appeared at line {names[name]})")

		rest = line[name_match.end():].strip().split()
		if 1 + len(rest) != COL_COUNT:
			errors.append(f"Line {i + 1}: {1 + len(rest)} columns (expected {COL_COUNT})")
			continue

		try:
			index = int(rest[0])
			if index not in indexes:
				indexes[index] = i
			else:
				errors.append(f"Line {i + 1}: {index} is duplicate index (first appeared at line {indexes[index]})")
		except ValueError:
			errors.append(f"Line {i + 1}: {rest[0]} is not an integer")

		try:
			enum = int(rest[1], 0)
			if enum not in enums:
				enums[enum] = i
			else:
				errors.append(f"Line {i + 1}: {enum} is duplicate enum (first appeared at line {enums[enum]})")
		except ValueError:
			errors.append(f"Line {i + 1}: {rest[1]} is not an integer")

		comments[i] = comment.strip()

	row_len = len(indexes)
	if row_len == 0:
		errors.append(f"No rows!")

	if any(index not in indexes for index in range(row_len)):
		errors.append(f"Index set is not a range({row_len})")

	if len(index_type) == 0 or len(enum_type) == 0:
		errors.append(f"Missing or malformed ;-metadata line")

	if len(errors) > 0:
		return errors

	rows = [EnumRow() for _ in range(row_len)]
	lut = {line: index for index, line in indexes.items()}

	for name, i in names.items():
		rows[lut[i]].name = name

	for enum, i in enums.items():
		rows[lut[i]].enum = enum

	for i, comment in comments.items():
		rows[lut[i]].comment = comment

	build(enum_file, rows, index_type, enum_type, rows[next(iter(indexes))].enum)
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
