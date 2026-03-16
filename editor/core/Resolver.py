GROUP_OPEN = '['
GROUP_CLOSE = ']'
MACRO_PREFIX = '$'
MACRO_END = '^'


def _get_temp(key: str):
	from .commands.temp import Storage
	return Storage.get_temp(key)


def is_valid_macro_key(key: str) -> bool:
	for c in key:
		if not c.isalpha() and c != '-' and c != '_':
			return False
	else:
		return True


def expand_macros(argline: str, seen=None) -> str:
	if seen is None:
		seen = set()

	result = ""
	i = 0
	while i < len(argline):
		char = argline[i]
		if char != MACRO_PREFIX:
			if char != MACRO_END:
				result += char
			i += 1
			continue

		if i + 1 < len(argline) and argline[i + 1] == GROUP_OPEN:
			j = i + 2
			key = ""
			while j < len(argline) and argline[j] != GROUP_CLOSE:
				key += argline[j]
				j += 1
			if key in seen:
				raise ValueError(f"Cyclic macro detected: {key}")
			value = _get_temp(key)
			inner_seen = seen.copy()
			inner_seen.add(key)
			expanded = expand_macros(value, inner_seen)
			result += expanded
			i = j + 1
		else:
			j = i + 1
			key = ""
			while j < len(argline):
				if is_valid_macro_key(argline[j]) or argline[j] == GROUP_CLOSE:
					key += argline[j]
					j += 1
				else:
					if argline[j] == MACRO_END:
						j += 1
					break
			if not key:
				i += 1
				continue
			if key in seen:
				raise ValueError(f"Cyclic macro detected: {key}")
			value = _get_temp(key)
			inner_seen = seen.copy()
			inner_seen.add(key)
			expanded = expand_macros(value, inner_seen)
			result += expanded
			i = j

	return result


def split_groups(argline: str) -> list[str]:
	args: list[str] = []

	current = ''
	in_group = False

	for char in argline:
		if char == GROUP_OPEN:
			if in_group:
				current += char
			else:
				in_group = True
		elif char == GROUP_CLOSE:
			if in_group:
				in_group = False
				args.append(current)
				current = ''
			else:
				current += char
		elif char.isspace() and not in_group:
			if current:
				args.append(current)
				current = ''
		else:
			current += char

	if current:
		args.append(current)

	return args
