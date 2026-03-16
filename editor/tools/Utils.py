import sys


def eprint(*values: object, sep: str | None = " ", end: str | None = "\n"):
	print(*values, sep=sep, end=end, file=sys.stderr)
