from typing import override, Optional

import toml
from PySide6.QtCore import QSize, QObject, QEvent
from PySide6.QtGui import QKeyEvent
from PySide6.QtWidgets import QApplication

from editor import ui
from editor.core import MainWindow, InputSignalPathItem
from editor.core.MainTabHolder import EditorTab
from editor.core.common import Alerts
from editor.core.common.SettingsForm import handle_all_children_modification
from editor.core.tabs.signal import Signal
from editor.core.tabs.signal.KeyMap import KEY_MAP
from editor.core.tabs.signal.Signal import InputType


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

		self.scratch_signals: list[Signal.EditorSignal] = []
		self.scratch_mappings: list[dict] = []
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

		self.last_signal_index = -1
		self.ui.newSignal.clicked.connect(self.new_signal)
		self.ui.deleteSignal.clicked.connect(self.delete_signal)
		self.ui.selectSignal.currentIndexChanged.connect(self.select_signal)
		self.ui.signalName.editingFinished.connect(self.signal_name_changed)
		self.ui.signalName.returnPressed.connect(lambda: self.ui.signalName.clearFocus())

		self.revert_changes_impl()
		handle_all_children_modification(self, lambda: self.set_asterisk(True))

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
		if len(self.scratch_signals) > 0:
			self.scratch_signals[self.ui.selectSignal.currentIndex()] = self.convert_signal_from_ui()
		signals = []
		for signal in self.scratch_signals:
			signals.append(Signal.convert_signal_from_editor_to_oly_format(signal).to_dict())

		# TODO v3 update mappings
		mappings = []

		content = {'header': 'signal', 'signal': signals, 'mapping': mappings}
		with open(self.item.full_path, 'w') as f:
			toml.dump(content, f)

	@override
	def revert_changes_impl(self):
		with open(self.item.full_path, 'r') as f:
			content = toml.load(f)

		self.ui.selectSignal.clear()
		self.disable_signal_page()

		self.scratch_signals.clear()
		signals = content['signal'] if 'signal' in content else []

		if len(signals) > 0:
			self.enable_signal_page()
			self.ui.selectSignal.blockSignals(True)
			for i in range(len(signals)):
				self.scratch_signals.append(
					Signal.convert_signal_from_oly_to_editor_format(Signal.OlySignal.from_dict(signals[i])))
				self.ui.selectSignal.addItem(self.scratch_signals[-1].basic.name)
			self.ui.selectSignal.setCurrentIndex(0)
			self.last_signal_index = 0
			self.ui.selectSignal.blockSignals(False)
			self.convert_signal_to_ui(self.scratch_signals[0])

		self.scratch_mappings.clear()
		mappings = content['mapping'] if 'mapping' in content else []
		# TODO v3 do the same for mappings

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
		self._set_widget_visible(self.ui.signalDeadzoneLabel,
								 stype in (InputType.GAMEPAD_1D_AXIS, InputType.GAMEPAD_2D_AXIS))
		self._set_widget_visible(self.ui.signalDeadzone,
								 stype in (InputType.GAMEPAD_1D_AXIS, InputType.GAMEPAD_2D_AXIS))
		# TODO v3 FIX change from Key -> Gamepad 1D Axis; the spacer doesn't appear. sometimes the post swizzle spacer doesn't appear either.
		self.ui.preDeadzoneSpacer.changeSize(0, 10 if stype in (InputType.GAMEPAD_1D_AXIS,
																InputType.GAMEPAD_2D_AXIS) else 0)

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
		self._set_widget_visible(self.ui.signalModsGroupBox, stype in (InputType.KEY,
																	   InputType.MOUSE_BUTTON))  # TODO v3 support for cursor pos and scroll as well

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

	def new_signal(self):
		index = self.ui.selectSignal.count()
		self.ui.selectSignal.blockSignals(True)
		self.ui.selectSignal.addItem(f"New Signal ({index})")
		self.ui.selectSignal.setCurrentIndex(index)
		self.ui.selectSignal.blockSignals(False)

		self.scratch_signals.append(
			Signal.EditorSignal(basic=Signal.BasicSection(name=self.ui.signalName.text(), type=InputType.KEY, key=32)))
		self.enable_signal_page()
		self.convert_signal_to_ui(self.scratch_signals[index])

	def delete_signal(self):
		index = self.ui.selectSignal.currentIndex()
		self.scratch_signals.pop(index)
		self.ui.selectSignal.blockSignals(True)
		self.ui.selectSignal.removeItem(index)
		self.ui.selectSignal.blockSignals(False)
		if self.ui.selectSignal.count() == 0:
			self.disable_signal_page()
		else:
			self.convert_signal_to_ui(self.scratch_signals[self.ui.selectSignal.currentIndex()])

	def enable_signal_page(self):
		self.ui.selectSignal.setDisabled(False)
		self.ui.deleteSignal.setDisabled(False)
		self.ui.signalBasicGroupBox.setDisabled(False)
		self.ui.signalModsGroupBox.setDisabled(False)
		self.ui.signalConversionGroupBox.setDisabled(False)

	def disable_signal_page(self):
		self.convert_signal_to_ui(Signal.EditorSignal(basic=Signal.BasicSection(name="", type=InputType.KEY, key=32)))
		self.ui.selectSignal.setDisabled(True)
		self.ui.deleteSignal.setDisabled(True)
		self.ui.signalBasicGroupBox.setDisabled(True)
		self.ui.signalModsGroupBox.setDisabled(True)
		self.ui.signalConversionGroupBox.setDisabled(True)

	def select_signal(self):
		if self.last_signal_index >= 0:
			self.scratch_signals[self.last_signal_index] = self.convert_signal_from_ui()
		self.last_signal_index = self.ui.selectSignal.currentIndex()
		self.ui.signalName.blockSignals(True)
		self.ui.signalName.setText(self.ui.selectSignal.currentText())
		self.ui.signalName.blockSignals(False)
		self.convert_signal_to_ui(self.scratch_signals[self.last_signal_index])

	def signal_name_changed(self):
		name = self.ui.signalName.text()
		cont = True
		while cont:
			cont = False
			for i in range(self.ui.selectSignal.count()):
				if i != self.ui.selectSignal.currentIndex():
					if name == self.ui.selectSignal.itemText(i):
						name = f"{name}*"
						cont = True
						break

		self.ui.selectSignal.setItemText(self.ui.selectSignal.currentIndex(), name)
		self.ui.signalName.blockSignals(True)
		self.ui.signalName.setText(name)
		self.ui.signalName.blockSignals(False)

	def convert_signal_from_ui(self) -> Signal.EditorSignal:
		signal = Signal.EditorSignal(
			Signal.BasicSection(name=self.ui.signalName.text(), type=self.ui.signalTypeSelect.currentIndex()))

		match signal.basic.type:
			case InputType.KEY:
				signal.basic.key = self.ui.keySelectSpinBox.value()
			case InputType.MOUSE_BUTTON:
				signal.basic.button = self.ui.mouseButtonSelect.currentIndex()
			case InputType.GAMEPAD_BUTTON:
				signal.basic.button = self.ui.gamepadButtonSelect.currentIndex()
			case InputType.GAMEPAD_1D_AXIS:
				signal.basic.axis1d = self.ui.gamepad1DAxisSelect.currentIndex()
				signal.basic.deadzone = self.ui.signalDeadzone.value()
			case InputType.GAMEPAD_2D_AXIS:
				signal.basic.axis2d = self.ui.gamepad2DAxisSelect.currentIndex()
				signal.basic.deadzone = self.ui.signalDeadzone.value()

		if signal.basic.type in (InputType.KEY, InputType.MOUSE_BUTTON):
			signal.mods = Signal.ModSection(
				ctrl=self.ui.keyModCtrl.currentIndex(),
				shift=self.ui.keyModShift.currentIndex(),
				alt=self.ui.keyModAlt.currentIndex()
			)

		signal.conversion.multiplier = [self.ui.multiplierX.value(), self.ui.multiplierY.value(),
										self.ui.multiplierZ.value()]
		signal.conversion.invert = [self.ui.invertX.isChecked(), self.ui.invertY.isChecked(),
									self.ui.invertZ.isChecked()]
		match signal.basic.type:
			case InputType.KEY | InputType.MOUSE_BUTTON | InputType.GAMEPAD_BUTTON:
				signal.conversion.dim = self.ui.dimensionConversion0D.currentIndex()
			case InputType.GAMEPAD_1D_AXIS:
				signal.conversion.dim = self.ui.dimensionConversion1D.currentIndex()
			case InputType.GAMEPAD_2D_AXIS | InputType.CURSOR_POSITION | InputType.SCROLL:
				signal.conversion.dim = self.ui.dimensionConversion2D.currentIndex()
		if self.ui.swizzle2D.isVisible():
			signal.conversion.swizzle = self.ui.swizzle2D.currentText()
		elif self.ui.swizzle3D.isVisible():
			signal.conversion.swizzle = self.ui.swizzle3D.currentText()

		return signal

	def convert_signal_to_ui(self, signal: Signal.EditorSignal):
		self.ui.signalName.setText(signal.basic.name)
		self.signal_name_changed()
		self.ui.signalTypeSelect.setCurrentIndex(signal.basic.type)
		match signal.basic.type:
			case InputType.KEY:
				self.ui.keySelectSpinBox.setValue(signal.basic.key)
			case InputType.MOUSE_BUTTON:
				self.ui.mouseButtonSelect.setCurrentIndex(signal.basic.button)
			case InputType.GAMEPAD_BUTTON:
				self.ui.gamepadButtonSelect.setCurrentIndex(signal.basic.button)
			case InputType.GAMEPAD_1D_AXIS:
				self.ui.gamepad1DAxisSelect.setCurrentIndex(signal.basic.axis1d)
				self.ui.signalDeadzone.setValue(signal.basic.deadzone)
			case InputType.GAMEPAD_2D_AXIS:
				self.ui.gamepad2DAxisSelect.setCurrentIndex(signal.basic.axis2d)
				self.ui.signalDeadzone.setValue(signal.basic.deadzone)

		if signal.mods is not None:
			self.ui.keyModCtrl.setCurrentIndex(signal.mods.ctrl)
			self.ui.keyModShift.setCurrentIndex(signal.mods.shift)
			self.ui.keyModAlt.setCurrentIndex(signal.mods.alt)

		match signal.basic.type:
			case InputType.KEY | InputType.MOUSE_BUTTON | InputType.GAMEPAD_BUTTON:
				self.ui.dimensionConversion0D.setCurrentIndex(signal.conversion.dim)
			case InputType.GAMEPAD_1D_AXIS:
				self.ui.dimensionConversion1D.setCurrentIndex(signal.conversion.dim)
			case InputType.GAMEPAD_2D_AXIS | InputType.CURSOR_POSITION | InputType.SCROLL:
				self.ui.dimensionConversion2D.setCurrentIndex(signal.conversion.dim)
		if self.ui.swizzle2D.isVisible():
			swizzle = signal.conversion.swizzle
			self.ui.swizzle2D.setCurrentText(swizzle if swizzle is not None else 'None')
		elif self.ui.swizzle3D.isVisible():
			swizzle = signal.conversion.swizzle
			self.ui.swizzle3D.setCurrentText(swizzle if swizzle is not None else 'None')
		self.ui.multiplierX.setValue(signal.conversion.multiplier[0])
		self.ui.multiplierY.setValue(signal.conversion.multiplier[1])
		self.ui.multiplierZ.setValue(signal.conversion.multiplier[2])
		self.ui.invertX.setChecked(signal.conversion.invert[0])
		self.ui.invertY.setChecked(signal.conversion.invert[1])
		self.ui.invertZ.setChecked(signal.conversion.invert[2])
