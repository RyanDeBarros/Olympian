from PySide6.QtGui import Qt


class KeyRepr:
	def __init__(self, qt_code, glfw_code, text, from_qt_keypad):
		self.qt_code = qt_code
		self.glfw_code = glfw_code
		self.text = text
		self.from_qt_keypad = from_qt_keypad


class _KeyMap:
	def __init__(self):
		self.from_qt = {}
		self.from_glfw = {}
		self.from_qt_keypad = {}

	def get_from_qt(self, qt_code, qt_mods) -> KeyRepr | None:
		if qt_mods & Qt.KeyboardModifier.KeypadModifier:
			return self.from_qt_keypad[qt_code]
		elif qt_code in self.from_qt:
			return self.from_qt[qt_code]
		else:
			return None

	def get_from_glfw(self, glfw_code) -> KeyRepr | None:
		return self.from_glfw[glfw_code] if glfw_code in self.from_glfw else None


KEY_MAP = _KeyMap()


def _register(qt_code, glfw_code, text):
	krepr = KeyRepr(qt_code, glfw_code, text, False)
	KEY_MAP.from_qt[qt_code] = krepr
	KEY_MAP.from_glfw[glfw_code] = krepr

def _register_from_keypad(qt_code, glfw_code, text):
	krepr = KeyRepr(qt_code, glfw_code, text, True)
	KEY_MAP.from_qt_keypad[qt_code] = krepr
	KEY_MAP.from_glfw[glfw_code] = krepr


_register(Qt.Key.Key_Space, 32, "[SPACE]")
_register(Qt.Key.Key_Apostrophe, 39, "'")
_register(Qt.Key.Key_Comma, 44, ",")
_register(Qt.Key.Key_Minus, 45, "-")
_register(Qt.Key.Key_Period, 46, ".")
_register(Qt.Key.Key_Slash, 47, "/")
for i in range(10):
	_register(Qt.Key.Key_0 + i, 48 + i, f"{i}")
_register(Qt.Key.Key_Semicolon, 59, ";")
_register(Qt.Key.Key_Equal, 61, "=")
for i in range(26):
	_register(Qt.Key.Key_A + i, 65 + i, chr(65 + i))
_register(Qt.Key.Key_BracketLeft, 91, "[")
_register(Qt.Key.Key_Backslash, 92, "\\")
_register(Qt.Key.Key_BracketRight, 93, "]")
_register(Qt.Key.Key_QuoteLeft, 96, "`")

_register(Qt.Key.Key_Escape, 256, "[ESC]")
_register(Qt.Key.Key_Enter, 257, "[ENTER]")
_register(Qt.Key.Key_Return, 257, "[ENTER]")
_register(Qt.Key.Key_Tab, 258, "[TAB]")
_register(Qt.Key.Key_Backspace, 259, "[BACKSPACE]")
_register(Qt.Key.Key_Insert, 260, "[INS]")
_register(Qt.Key.Key_Delete, 261, "[DEL]")
_register(Qt.Key.Key_Right, 262, "[RIGHT]")
_register(Qt.Key.Key_Left, 263, "[LEFT]")
_register(Qt.Key.Key_Down, 264, "[DOWN]")
_register(Qt.Key.Key_Up, 265, "[UP]")
_register(Qt.Key.Key_PageUp, 266, "[PAGE-UP]")
_register(Qt.Key.Key_PageDown, 267, "[PAGE-DOWN]")
_register(Qt.Key.Key_Home, 268, "[HOME]")
_register(Qt.Key.Key_End, 269, "[END]")
_register(Qt.Key.Key_CapsLock, 280, "[CAPS-LOCK]")
_register(Qt.Key.Key_ScrollLock, 281, "[SCROLL-LOCK]")
_register(Qt.Key.Key_NumLock, 282, "[NUM-LOCK]")
_register(Qt.Key.Key_Print, 283, "[PRTSCN]")
_register(Qt.Key.Key_Pause, 284, "[PAUSE]")

for i in range(25):
	_register(Qt.Key.Key_F1 + i, 290 + i, f"[F{1 + i}]")
for i in range(10):
	_register_from_keypad(Qt.Key.Key_0 + i, 320 + i, f"[KP {i}]")

_register_from_keypad(Qt.Key.Key_Period, 330, "[KP .]")
_register_from_keypad(Qt.Key.Key_Slash, 331, "[KP /]")
_register_from_keypad(Qt.Key.Key_Asterisk, 332, "[KP *]")
_register_from_keypad(Qt.Key.Key_Minus, 333, "[KP -]")
_register_from_keypad(Qt.Key.Key_Plus, 334, "[KP +]")
_register_from_keypad(Qt.Key.Key_Enter, 335, "[KP ENTER]")
_register_from_keypad(Qt.Key.Key_Return, 335, "[KP ENTER]")
_register_from_keypad(Qt.Key.Key_Equal, 336, "[KP =]")

_register(Qt.Key.Key_Shift, 340, "[LEFT-SHIFT]")
_register(Qt.Key.Key_Control, 341, "[LEFT-CTRL]")
_register(Qt.Key.Key_Alt, 342, "[LEFT-ALT]")
_register(Qt.Key.Key_Super_L, 343, "[LEFT-SUPER]")
_register(Qt.Key.Key_Super_R, 347, "[RIGHT-SUPER]")

