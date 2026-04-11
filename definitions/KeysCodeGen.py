import argparse
from pathlib import Path

from . import CodeGen

ENGINE_DIR = Path("engine")
DEFINITIONS_DIR = Path("definitions")
KEYS_DIR = DEFINITIONS_DIR / "keys"
GEN_ROOT_DIR = ENGINE_DIR / ".gen"
GEN_KEYS_DIR = GEN_ROOT_DIR / "keys"


def gen(keys_file: Path, *args, **kwargs) -> list[str]:
	max_chars: int = kwargs["max_chars"]
	underlying_type: str = kwargs["underlying_type"]

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

			if len(code) > max_chars:
				bad_lines[i] = f"more than {max_chars} chars"
				continue

			padded_code = code.ljust(max_chars, '\0')
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
	enum class {keys_file.stem} : {underlying_type}
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
	parser = argparse.ArgumentParser()
	parser.add_argument("--key-size", type=int, choices=[4, 8])
	args = parser.parse_args()

	codegen = CodeGen("keys")
	codegen.process(gen, None, max_chars=args.key_size, underlying_type="unsigned int" if args.key_size == 4 else "unsigned long long")
