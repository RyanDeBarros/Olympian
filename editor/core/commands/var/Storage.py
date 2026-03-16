# TODO v7 persistent storage alongside temp

temp: dict[str, str] = {}


def get_temp(key: str) -> str:
	global temp
	return temp[key]


def set_temp(key: str, value: str):
	global temp
	temp[key] = value


def del_temp(key: str):
	global temp
	if key in temp:
		del temp[key]


def temp_keys() -> list[str]:
	return list(temp.keys())
