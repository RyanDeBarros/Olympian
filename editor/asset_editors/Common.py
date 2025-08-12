from typing import List

from PySide6.QtWidgets import QComboBox, QCheckBox, QSpinBox, QDoubleSpinBox

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

	@staticmethod
	def get_value(control):
		if isinstance(control, QComboBox):
			return PARAM_LIST.get_value(control.currentText())
		elif isinstance(control, QCheckBox):
			return control.isChecked()
		elif isinstance(control, QSpinBox):
			return control.value()
		elif isinstance(control, QDoubleSpinBox):
			return control.value()
		else:
			return None

	def get_dict(self):
		d = {}
		for name, control in self.params.items():
			value = SettingsForm.get_value(control)
			if value is not None:
				d[name] = value
		return d

	@staticmethod
	def set_value(control, value):
		if isinstance(control, QComboBox):
			control.setCurrentText(PARAM_LIST.get_name(value))
		elif isinstance(control, QCheckBox):
			control.setChecked(value)
		elif isinstance(control, QSpinBox):
			control.setValue(value)
		elif isinstance(control, QDoubleSpinBox):
			control.setValue(value)

	def load_dict(self, d, defaults=None):
		if defaults is not None:
			fit_with_defaults(d, defaults)

		for name, value in d.items():
			if name in self.params:
				SettingsForm.set_value(self.params[name], value)
