import sys
from pathlib import Path

ENGINE_DIR = Path("engine")
DEFINITIONS_DIR = Path("definitions")
KEYS_FILE = DEFINITIONS_DIR / "OlympianEngine.keys"
GEN_ROOT_DIR = ENGINE_DIR / ".gen"
GEN_INL_PATH = GEN_ROOT_DIR / "AssetKeys.inl"

MAX_CHARS = 4


# TODO v7 use keys folder similar to enums folder so new keys can be added without recompiling unrelated files


if __name__ == "__main__":
	enums: dict[str, dict[str, str]] = {}
	bad_lines: dict[int, str] = {}

	current_enum = None

	with open(KEYS_FILE, 'r') as f:
		for i, line in enumerate(f):
			line = line.strip()
			if not line:
				continue

			if line.startswith("[") and line.endswith("]"):
				current_enum = line[1:-1].strip()
				if current_enum not in enums:
					enums[current_enum] = {}
				continue

			if current_enum is None:
				bad_lines[i] = "entry not inside an enum section"
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
			if padded_code in enums[current_enum]:
				bad_lines[i] = f"code {code} already registered in [{current_enum}]"
			else:
				enums[current_enum][padded_code] = name

	if len(bad_lines) > 0:
		for i, error in bad_lines.items():
			print(f"Keys code generation failed for line {i}: {error}")
		sys.exit(1)

	enum_blocks = ""
	for enum_name, entries in enums.items():
		enum_def = ""
		for i, (code, name) in enumerate(entries.items()):
			value = int.from_bytes(code.encode())
			enum_def += f"\t\t{name} = {value}"
			if i + 1 < len(entries):
				enum_def += ",\n"

		enum_blocks += f"""
\tenum class {enum_name} : unsigned int
\t{{
{enum_def}
\t}};
"""

	codegen = f"""namespace oly::_gen::keys
{{{enum_blocks}}}
"""

	GEN_INL_PATH.write_text(codegen)
