import os.path

import toml


def meta(path):
	"""Peek the first line to extract meta fields."""
	with open(path, 'r') as f:
		first_line = f.readline()
	m = {}
	if first_line.startswith('#meta'):
		fields = first_line[len('#meta'):].strip().split()
		for field in fields:
			field = field.split('=', 1)
			if len(field) == 2:
				k, v = field
				m[k] = v.strip('"')
	return m


def load(path):
	"""Load TOML data, skipping the meta line if present."""
	with open(path, 'r') as f:
		first_line = f.readline()
		if not first_line.startswith('#meta'):
			f.seek(0)
		return toml.load(f)


def dump(path, d, m=None):
	"""Dump TOML data with optional meta line at the top, merging with existing meta."""
	if os.path.exists(path):
		existing_meta = meta(path)
	else:
		existing_meta = {}
	if m is not None:
		existing_meta.update(m)

	with open(path, 'w') as f:
		if existing_meta:
			meta_line = "#meta"
			for k, v in existing_meta.items():
				meta_line += f' {k}="{v}"'
			f.write(meta_line + '\n\n')
		toml.dump(d, f)
