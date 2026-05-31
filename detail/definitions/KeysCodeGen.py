import argparse
from pathlib import Path

import toml
from toml import TomlDecodeError

from . import CodeGen

ENGINE_DIR = Path("engine")
DEFINITIONS_DIR = Path("definitions")
KEYS_DIR = DEFINITIONS_DIR / "keys"
GEN_ROOT_DIR = ENGINE_DIR / ".gen"
GEN_KEYS_DIR = GEN_ROOT_DIR / "keys"


# TODO v7 codegen for meta fields, specifically asset types


def gen(keys_file: Path, *args, **kwargs) -> list[str]:
	max_chars: int = kwargs["max_chars"]
	underlying_type: str = kwargs["underlying_type"]

	try:
		d = toml.loads(keys_file.read_text())
	except TomlDecodeError as e:
		return [str(e)]

	bad_lines: dict[int, str] = {}
	keys: dict[str, str] = {}

	for i, key in enumerate(d['key']):
		try:
			label = str(key['label'])
			code = str(key['code'])
			enum = str(key['enum'])
			tooltip = str(key.get('tooltip', ''))
		except (KeyError, ValueError) as e:
			bad_lines[i] = repr(e)
			continue

		if len(code) > max_chars:
			bad_lines[i] = f"more than {max_chars} chars"
			continue

		padded_code = code.ljust(max_chars, '\0')
		if padded_code in keys:
			bad_lines[i] = f"code {code} already registered"
		else:
			keys[padded_code] = enum

	if len(bad_lines) > 0:
		return [f"Key #{i + 1}: {error}" for i, error in bad_lines.items()]

	enum_def = ""
	for i, (code, name) in enumerate(keys.items()):
		value = int.from_bytes(code.encode())
		enum_def += f"\t\t{name} = {value}"
		if i + 1 < len(keys):
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
	codegen.process(gen, "*.toml", max_chars=args.key_size, underlying_type="unsigned int" if args.key_size == 4 else "unsigned long long")
