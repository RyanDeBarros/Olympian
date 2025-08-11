from typing import List

from PySide6.QtWidgets import QComboBox, QCheckBox, QSpinBox

from editor.util import *


class SettingsParameter:
	def __init__(self, name, control):
		self.name = name
		self.control = control


def fit_with_defaults(d, defaults):
	for k in defaults:
		d[k] = d.get(k, defaults[k])


class SettingsForm:
	def __init__(self, params: List[SettingsParameter]):
		self.params = {p.name: p.control for p in params}

	def get_dict(self):
		d = {}
		for name, control in self.params.items():
			if isinstance(control, QComboBox):
				d[name] = PARAM_LIST.get_value(control.currentText())
			elif isinstance(control, QCheckBox):
				d[name] = control.isChecked()
			elif isinstance(control, QSpinBox):
				d[name] = control.value()
		return d

	def load_dict(self, d, defaults=None):
		if defaults is not None:
			fit_with_defaults(d, defaults)

		for name, value in d.items():
			control = self.params[name]
			if isinstance(control, QComboBox):
				control.setCurrentText(PARAM_LIST.get_name(value))
			elif isinstance(control, QCheckBox):
				control.setChecked(value)
			elif isinstance(control, QSpinBox):
				control.setValue(value)
