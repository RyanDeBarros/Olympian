from enum import IntEnum
from typing import override, Optional

import toml
from PySide6.QtCore import QSize, QObject, QEvent
from PySide6.QtGui import QKeyEvent
from PySide6.QtWidgets import QApplication

from editor import ui
from editor.core import MainWindow, InputSignalPathItem
from editor.core.MainTabHolder import EditorTab
from editor.core.common import Alerts
from editor.core.tabs.KeyMap import KEY_MAP


class InputType(IntEnum):
	KEY = 0
	MOUSE_BUTTON = 1
	GAMEPAD_BUTTON = 2
	GAMEPAD_1D_AXIS = 3
	GAMEPAD_2D_AXIS = 4
	CURSOR_POSITION = 5
	SCROLL = 6


class KeyCaptureFilter(QObject):
	def __init__(self, callback):
		super().__init__()
		self.callback = callback
		self.captured = False

	def eventFilter(self, obj, event):
		if event.type() == QEvent.Type.KeyPress and isinstance(event, QKeyEvent) and not self.captured:
			self.captured = True
			self.callback(event)
			return True
		return False


class InputSignalTab(EditorTab):
	def __init__(self, win: MainWindow, item: InputSignalPathItem):
		super().__init__(win)
		self.item = item

		self.content = {}
		self.ui = ui.InputSignal.Ui_InputSignal()
		self.ui.setupUi(self)

		self.ui.signalBasicForm.setVerticalSpacing(0)
		self.ui.signalConversionForm.setVerticalSpacing(0)
		self.ui.signalTypeSelect.currentIndexChanged.connect(self.signal_type_changed)
		self.ui.dimensionConversion0D.currentIndexChanged.connect(self.conversion_0d_changed)
		self.ui.dimensionConversion1D.currentIndexChanged.connect(self.conversion_1d_changed)
		self.ui.dimensionConversion2D.currentIndexChanged.connect(self.conversion_2d_changed)

		self.key_capture_filter: Optional[KeyCaptureFilter] = None
		self.destroyed.connect(lambda: QApplication.instance().removeEventFilter(self.key_capture_filter))
		self.ui.keyListenButton.clicked.connect(self.start_listening_for_key)
		self.ui.keySelectSpinBox.valueChanged.connect(self.key_select_spinbox_changed)
		self.key_select_spinbox_changed()

		self.revert_changes_impl()

	@override
	def uid(self):
		return self.item.full_path

	@override
	def icon(self, size: QSize):
		return self.item.icon(size)

	@override
	def name(self):
		return self.item.ui_name()

	@override
	def save_changes_impl(self):
		with open(self.item.full_path, 'w') as f:
			toml.dump(self.content, f)

	@override
	def revert_changes_impl(self):
		with open(self.item.full_path, 'r') as f:
			self.content = toml.load(f)
		self.signal_type_changed()

	@override
	def rename_impl(self, item: InputSignalPathItem):
		assert isinstance(item, InputSignalPathItem)
		self.item = item

	@staticmethod
	def _set_widget_visible(widget, visible):
		if visible:
			widget.show()
		else:
			widget.hide()

	@staticmethod
	def _set_layout_visible(layout, visible):
		for i in range(layout.count()):
			InputSignalTab._set_widget_visible(layout.itemAt(i).widget(), visible)

	def signal_type_changed(self):
		stype = self.ui.signalTypeSelect.currentIndex()
		self._set_widget_visible(self.ui.signalKeyLabel, stype == InputType.KEY)
		self._set_layout_visible(self.ui.signalKeyLayout, stype == InputType.KEY)
		self._set_widget_visible(self.ui.signalMouseButtonLabel, stype == InputType.MOUSE_BUTTON)
		self._set_layout_visible(self.ui.signalMouseButtonLayout, stype == InputType.MOUSE_BUTTON)
		self._set_widget_visible(self.ui.signalGamepadButtonLabel, stype == InputType.GAMEPAD_BUTTON)
		self._set_layout_visible(self.ui.signalGamepadButtonLayout, stype == InputType.GAMEPAD_BUTTON)
		self._set_widget_visible(self.ui.signalGamepad1DAxisLabel, stype == InputType.GAMEPAD_1D_AXIS)
		self._set_layout_visible(self.ui.signalGamepad1DAxisLayout, stype == InputType.GAMEPAD_1D_AXIS)
		self._set_widget_visible(self.ui.signalGamepad2DAxisLabel, stype == InputType.GAMEPAD_2D_AXIS)
		self._set_layout_visible(self.ui.signalGamepad2DAxisLayout, stype == InputType.GAMEPAD_2D_AXIS)
		self._set_widget_visible(self.ui.dimensionConversionLabel0D,
								 stype in (InputType.KEY, InputType.MOUSE_BUTTON, InputType.GAMEPAD_BUTTON))
		if not self.ui.dimensionConversion0D.isVisible() and stype in (InputType.KEY, InputType.MOUSE_BUTTON,
																	   InputType.GAMEPAD_BUTTON):
			self.conversion_0d_changed()
		self._set_widget_visible(self.ui.dimensionConversion0D,
								 stype in (InputType.KEY, InputType.MOUSE_BUTTON, InputType.GAMEPAD_BUTTON))
		self._set_widget_visible(self.ui.dimensionConversionLabel1D, stype == InputType.GAMEPAD_1D_AXIS)
		if not self.ui.dimensionConversion1D.isVisible() and stype == InputType.GAMEPAD_1D_AXIS:
			self.conversion_1d_changed()
		self._set_widget_visible(self.ui.dimensionConversion1D, stype == InputType.GAMEPAD_1D_AXIS)
		self._set_widget_visible(self.ui.dimensionConversionLabel2D,
								 stype in (InputType.GAMEPAD_2D_AXIS, InputType.CURSOR_POSITION, InputType.SCROLL))
		if not self.ui.dimensionConversion2D.isVisible() and stype in (InputType.GAMEPAD_2D_AXIS,
																	   InputType.CURSOR_POSITION, InputType.SCROLL):
			self.conversion_2d_changed()
		self._set_widget_visible(self.ui.dimensionConversion2D,
								 stype in (InputType.GAMEPAD_2D_AXIS, InputType.CURSOR_POSITION, InputType.SCROLL))

	def conversion_0d_changed(self):
		ctype = self.ui.dimensionConversion0D.currentIndex()
		self._set_widget_visible(self.ui.swizzle2DLabel, ctype == 2)
		self._set_widget_visible(self.ui.swizzle2D, ctype == 2)
		self._set_widget_visible(self.ui.swizzle3DLabel, ctype == 3)
		self._set_widget_visible(self.ui.swizzle3D, ctype == 3)
		self.ui.postSwizzleSpacer.changeSize(0, 10 if ctype in (2, 3) else 0)
		self.ui.multiplierX.setDisabled(ctype <= 0)
		self.ui.multiplierXlabel.setDisabled(ctype <= 0)
		self.ui.multiplierY.setDisabled(ctype <= 1)
		self.ui.multiplierYlabel.setDisabled(ctype <= 1)
		self.ui.multiplierZ.setDisabled(ctype <= 2)
		self.ui.multiplierZlabel.setDisabled(ctype <= 2)
		self.ui.invertY.setDisabled(ctype <= 1)
		self.ui.invertZ.setDisabled(ctype <= 2)

	def conversion_1d_changed(self):
		ctype = self.ui.dimensionConversion1D.currentIndex()
		self._set_widget_visible(self.ui.swizzle2DLabel, ctype == 2)
		self._set_widget_visible(self.ui.swizzle2D, ctype == 2)
		self._set_widget_visible(self.ui.swizzle3DLabel, ctype == 3)
		self._set_widget_visible(self.ui.swizzle3D, ctype == 3)
		self.ui.postSwizzleSpacer.changeSize(0, 10 if ctype in (2, 3) else 0)
		self.ui.multiplierX.setDisabled(ctype == 1)
		self.ui.multiplierXlabel.setDisabled(ctype == 1)
		self.ui.multiplierY.setDisabled(ctype <= 1)
		self.ui.multiplierYlabel.setDisabled(ctype <= 1)
		self.ui.multiplierZ.setDisabled(ctype <= 2)
		self.ui.multiplierZlabel.setDisabled(ctype <= 2)
		self.ui.invertY.setDisabled(ctype <= 1)
		self.ui.invertZ.setDisabled(ctype <= 2)

	def conversion_2d_changed(self):
		ctype = self.ui.dimensionConversion2D.currentIndex()
		self._set_widget_visible(self.ui.swizzle2DLabel, ctype == 0)
		self._set_widget_visible(self.ui.swizzle2D, ctype == 0)
		self._set_widget_visible(self.ui.swizzle3DLabel, ctype in (7, 8))
		self._set_widget_visible(self.ui.swizzle3D, ctype in (7, 8))
		self.ui.postSwizzleSpacer.changeSize(0, 10 if ctype in (0, 7, 8) else 0)
		self.ui.multiplierX.setDisabled(0 < ctype <= 3)
		self.ui.multiplierXlabel.setDisabled(0 < ctype <= 3)
		self.ui.multiplierY.setDisabled(0 < ctype <= 6)
		self.ui.multiplierYlabel.setDisabled(0 < ctype <= 6)
		self.ui.multiplierZ.setDisabled(0 <= ctype <= 6)
		self.ui.multiplierZlabel.setDisabled(0 <= ctype <= 6)
		self.ui.invertY.setDisabled(0 < ctype <= 6)
		self.ui.invertZ.setDisabled(0 <= ctype <= 6)

	def start_listening_for_key(self):
		self.key_capture_filter = KeyCaptureFilter(self.key_captured)
		QApplication.instance().installEventFilter(self.key_capture_filter)

	def key_captured(self, key: QKeyEvent):
		QApplication.instance().removeEventFilter(self.key_capture_filter)
		self.key_capture_filter = None
		k = KEY_MAP.get_from_qt(key.key(), key.modifiers())
		if k is not None:
			self.ui.keySelectDisplay.setText(k.text)
			self.ui.keySelectSpinBox.blockSignals(True)
			self.ui.keySelectSpinBox.setValue(k.glfw_code)
			self.ui.keySelectSpinBox.blockSignals(False)
		else:
			Alerts.alert_error(self, "Unrecognized key", "Please manually enter the GLFW key code")

	def key_select_spinbox_changed(self):
		k = KEY_MAP.get_from_glfw(self.ui.keySelectSpinBox.value())
		if k is not None:
			self.ui.keySelectDisplay.setText(k.text)
		else:
			self.ui.keySelectDisplay.setText("Unrecognized key code")
