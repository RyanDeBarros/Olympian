import sys
from pathlib import Path


MAX_CHARS = 8


if __name__ == "__main__":
	bad_lines: dict[int, str] = {}
	lines: list[tuple[str, int]] = []

	i = 0
	with open("Keys.enum", 'r') as f:
		for line in f:
			i += 1
			if '=' not in line:
				continue

			name, value = (x.strip() for x in line.split('=', 1))

			if len(value) > MAX_CHARS:
				bad_lines[i] = f"more than {MAX_CHARS} chars"
				continue

			padded_code = int.from_bytes(value.ljust(MAX_CHARS, '\0').encode())
			lines.append((name, padded_code))

	if bad_lines:
		for line, error in bad_lines.items():
			print(f"KeysCodeGen error at line {line}: {error}")
		sys.exit(1)

	enum_def = ""
	for i, (name, value) in enumerate(lines):
		enum_def += f"\t\t{name} = {value}"
		if i + 1 < len(lines):
			enum_def += ",\n"

	codegen = f"""#pragma once

namespace oly::detail
{{
	enum class Key : unsigned long long
	{{
{enum_def}
	}};
}}
"""

	Path("Keys.h").write_text(codegen)
