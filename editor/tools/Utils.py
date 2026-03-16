import sys


def eprint(*values: object):
	print("Error:", *values, file=sys.stderr)
