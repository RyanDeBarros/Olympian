temp: dict[str, str] = {}


def get_temp(key: str):
	global temp
	return temp[key]


def set_temp(key: str, value: str):
	global temp
	temp[key] = value


def del_temp(key: str):
	global temp
	if key in temp:
		del temp[key]
