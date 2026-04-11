import json
import sys
from pathlib import Path

ENGINE_DIR = Path("engine")
DEFINITIONS_DIR = Path("definitions")
KEYS_DIR = DEFINITIONS_DIR / "keys"
GEN_ROOT_DIR = ENGINE_DIR / ".gen"
GEN_KEYS_DIR = GEN_ROOT_DIR / "keys"
CACHE_DIR = GEN_ROOT_DIR / ".cache"
CACHE_FILE = CACHE_DIR / "keys.json"

MAX_CHARS = 4


def load_cache() -> dict:
	cache = CACHE_FILE.read_text() if CACHE_FILE.exists() else ""
	return json.loads(cache) if len(cache) > 0 else {}


def save_cache(cache: dict) -> None:
	CACHE_FILE.parent.mkdir(parents=True, exist_ok=True)
	CACHE_FILE.touch(exist_ok=True)
	CACHE_FILE.write_text(json.dumps(cache))


def prune_cache(files_seen: set[str]):
	for inl_file in GEN_KEYS_DIR.rglob("*.inl"):
		cache_entry = inl_file.relative_to(GEN_KEYS_DIR).with_suffix("").as_posix()
		if cache_entry not in files_seen:
			inl_file.unlink()

	for path in sorted(GEN_KEYS_DIR.rglob("*"), reverse=True):
		if path.is_dir():
			try:
				path.rmdir()  # only works if empty
			except OSError:
				pass



def gen(keys_file: Path) -> list[str]:
	good_lines: dict[str, str] = {}
	bad_lines: dict[int, str] = {}

	with open(keys_file, 'r') as f:
		for i, line in enumerate(f):
			line = line.strip()
			if not line:
				continue

			try:
				code, name = line.split(maxsplit=1)
			except ValueError:
				bad_lines[i] = "invalid error"
				continue

			code = code.strip()
			name = name.strip()
			if not code or not name:
				continue

			if len(code) > MAX_CHARS:
				bad_lines[i] = f"more than {MAX_CHARS} chars"
				continue

			padded_code = code.rjust(MAX_CHARS, '\0')
			if padded_code in good_lines:
				bad_lines[i] = f"code {code} already registered"
			else:
				good_lines[padded_code] = name

	if len(bad_lines) > 0:
		return [f"Line {i + 1}: {error}" for i, error in bad_lines.items()]

	enum_def = ""
	for i, (code, name) in enumerate(good_lines.items()):
		value = int.from_bytes(code.encode())
		enum_def += f"\t\t{name} = {value}"
		if i + 1 < len(good_lines):
			enum_def += ",\n"

	codegen = f"""namespace oly::_gen::keys
{{
	enum class {keys_file.stem} : unsigned int
	{{
{enum_def}
	}};
}}
"""

	GEN_KEYS_DIR.mkdir(exist_ok=True, parents=True)
	gen_path = GEN_KEYS_DIR / keys_file.with_suffix(".inl").name
	gen_path.touch(exist_ok=True)
	gen_path.write_text(codegen)
	return []


if __name__ == "__main__":
	fail = False
	cache = load_cache()
	new_cache = {}
	files_seen: set[str] = set()

	for file in KEYS_DIR.iterdir():
		cache_entry = file.relative_to(KEYS_DIR).with_suffix("").as_posix()
		files_seen.add(cache_entry)

		mtime = file.stat().st_mtime

		if cache_entry in cache and cache[cache_entry] == mtime:
			new_cache[cache_entry] = mtime
			continue

		if file.is_file():
			errors = gen(file)
			if errors:
				for error in errors:
					print(f"Keys code generation failed for {file.relative_to(KEYS_DIR).as_posix()}: {error}")
				fail = True
			else:
				new_cache[cache_entry] = mtime

	prune_cache(files_seen)
	save_cache(new_cache)

	if fail:
		sys.exit(1)

# TODO v7 caching
