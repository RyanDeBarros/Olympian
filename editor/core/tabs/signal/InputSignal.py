from typing import override, Optional

import toml
from PySide6.QtCore import QSize, QObject, QEvent
from PySide6.QtGui import QKeyEvent, QIcon
from PySide6.QtWidgets import QApplication, QComboBox, QPushButton, QMessageBox, QHeaderView

from editor import ui
from editor.core import MainWindow, InputSignalPathItem, block_signals
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
		self.scratch_mappings: list[Signal.EditorMapping] = []
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

		self.last_mapping_index = -1
		self.ui.newMapping.clicked.connect(self.new_mapping)
		self.ui.deleteMapping.clicked.connect(self.delete_mapping)
		self.ui.selectMapping.currentIndexChanged.connect(self.select_mapping)
		self.ui.mappingName.editingFinished.connect(self.mapping_name_changed)
		self.ui.mappingName.returnPressed.connect(lambda: self.ui.mappingName.clearFocus())

		self.ui.mappingAppendSignal.clicked.connect(self.mapping_append_signal)
		self.ui.mappingClearSignals.clicked.connect(self.mapping_clear_signals)
		self.ui.mappingSignalTable.horizontalHeader().setSectionResizeMode(0, QHeaderView.ResizeMode.Stretch)
		self.ui.mappingSignalTable.horizontalHeader().setSectionResizeMode(1, QHeaderView.ResizeMode.Fixed)

		self.revert_changes_impl()
		handle_all_children_modification(self, lambda: self.set_asterisk(True))
		self.ui.newSignal.clicked.connect(lambda: self.set_asterisk(True))
		self.ui.deleteSignal.clicked.connect(lambda: self.set_asterisk(True))
		self.ui.newMapping.clicked.connect(lambda: self.set_asterisk(True))
		self.ui.deleteMapping.clicked.connect(lambda: self.set_asterisk(True))
		self.ui.mappingAppendSignal.clicked.connect(lambda: self.set_asterisk(True))
		self.ui.mappingClearSignals.clicked.connect(lambda: self.set_asterisk(True))

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

		if len(self.scratch_mappings) > 0:
			self.scratch_mappings[self.ui.selectMapping.currentIndex()] = self.convert_mapping_from_ui()
		mappings = []
		for mapping in self.scratch_mappings:
			mappings.append(Signal.convert_mapping_from_editor_to_oly_format(mapping).to_dict())

		content = {'header': 'signal', 'signal': signals, 'mapping': mappings}
		with open(self.item.full_path, 'w') as f:
			toml.dump(content, f)

	@override
	def revert_changes_impl(self):
		with open(self.item.full_path, 'r') as f:
			content = toml.load(f)

		with block_signals(self.ui.selectSignal) as selectSignal:
			selectSignal.clear()
		self.disable_signal_page()

		self.scratch_signals.clear()
		signals = content['signal'] if 'signal' in content else []

		if len(signals) > 0:
			self.enable_signal_page()
			with block_signals(self.ui.selectSignal) as selectSignal:
				for i in range(len(signals)):
					self.scratch_signals.append(
						Signal.convert_signal_from_oly_to_editor_format(Signal.OlySignal.from_dict(signals[i])))
					selectSignal.addItem(self.scratch_signals[-1].basic.name)
				selectSignal.setCurrentIndex(0)
				self.last_signal_index = 0
			self.convert_signal_to_ui(self.scratch_signals[0])

		self.signal_type_changed()

		self.scratch_mappings.clear()
		mappings = content['mapping'] if 'mapping' in content else []

		with block_signals(self.ui.selectMapping) as selectMapping:
			selectMapping.clear()
		self.disable_mapping_page()

		if len(mappings) > 0:
			self.enable_mapping_page()
			with block_signals(self.ui.selectMapping) as selectMapping:
				for i in range(len(mappings)):
					self.scratch_mappings.append(
						Signal.convert_mapping_from_oly_to_editor_format(Signal.OlyMapping.from_dict(mappings[i])))
					selectMapping.addItem(self.scratch_mappings[-1].name)
				selectMapping.setCurrentIndex(0)
				self.last_mapping_index = 0
			self.convert_mapping_to_ui(self.scratch_mappings[0])

	@override
	def rename_impl(self, item: InputSignalPathItem):
		assert isinstance(item, InputSignalPathItem)
		self.item = item

	@override
	def refresh_impl(self):
		pass

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

		key = stype == InputType.KEY
		self._set_widget_visible(self.ui.signalKeyLabel, key)
		self._set_layout_visible(self.ui.signalKeyLayout, key)

		mb = stype == InputType.MOUSE_BUTTON
		self._set_widget_visible(self.ui.signalMouseButtonLabel, mb)
		self._set_layout_visible(self.ui.signalMouseButtonLayout, mb)

		gpb = stype == InputType.GAMEPAD_BUTTON
		self._set_widget_visible(self.ui.signalGamepadButtonLabel, gpb)
		self._set_layout_visible(self.ui.signalGamepadButtonLayout, gpb)

		gp1a = stype == InputType.GAMEPAD_1D_AXIS
		self._set_widget_visible(self.ui.signalGamepad1DAxisLabel, gp1a)
		self._set_layout_visible(self.ui.signalGamepad1DAxisLayout, gp1a)

		gp2a = stype == InputType.GAMEPAD_2D_AXIS
		self._set_widget_visible(self.ui.signalGamepad2DAxisLabel, gp2a)
		self._set_layout_visible(self.ui.signalGamepad2DAxisLayout, gp2a)

		deadzone = stype in (InputType.GAMEPAD_1D_AXIS, InputType.GAMEPAD_2D_AXIS)
		self._set_widget_visible(self.ui.signalDeadzoneLabel, deadzone)
		self._set_widget_visible(self.ui.signalDeadzone, deadzone)
		# TODO v3 FIX change from Key -> Gamepad 1D Axis; the spacer doesn't appear. sometimes the post swizzle spacer doesn't appear either.
		self.ui.preDeadzoneSpacer.changeSize(0, 10 if deadzone else 0)

		dim0 = stype in (InputType.KEY, InputType.MOUSE_BUTTON, InputType.GAMEPAD_BUTTON)
		self._set_widget_visible(self.ui.dimensionConversionLabel0D, dim0)
		if not self.ui.dimensionConversion0D.isVisible() and dim0:
			self.conversion_0d_changed()
		self._set_widget_visible(self.ui.dimensionConversion0D, dim0)

		dim1 = stype == InputType.GAMEPAD_1D_AXIS
		self._set_widget_visible(self.ui.dimensionConversionLabel1D, dim1)
		if not self.ui.dimensionConversion1D.isVisible() and dim1:
			self.conversion_1d_changed()
		self._set_widget_visible(self.ui.dimensionConversion1D, dim1)

		dim2 = stype in (InputType.GAMEPAD_2D_AXIS, InputType.CURSOR_POSITION, InputType.SCROLL)
		self._set_widget_visible(self.ui.dimensionConversionLabel2D, dim2)
		if not self.ui.dimensionConversion2D.isVisible() and dim2:
			self.conversion_2d_changed()
		self._set_widget_visible(self.ui.dimensionConversion2D, dim2)

		self._set_widget_visible(self.ui.signalModsGroupBox, stype in (InputType.KEY, InputType.MOUSE_BUTTON))

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
			with block_signals(self.ui.keySelectSpinBox) as keySelectSpinBox:
				keySelectSpinBox.setValue(k.glfw_code)
		else:
			Alerts.alert_error(self, "Unrecognized key", "Please manually enter the GLFW key code")

	def key_select_spinbox_changed(self):
		k = KEY_MAP.get_from_glfw(self.ui.keySelectSpinBox.value())
		if k is not None:
			self.ui.keySelectDisplay.setText(k.text)
		else:
			self.ui.keySelectDisplay.setText("Unrecognized key code")

	def signal_exists(self, signal):
		for i in range(self.ui.selectSignal.count()):
			if i != self.ui.selectSignal.currentIndex():
				if signal == self.ui.selectSignal.itemText(i):
					return True
		return False

	def new_signal(self):
		index = self.ui.selectSignal.count()
		name = f"New Signal ({index})"
		while self.signal_exists(name):
			name = f"{name}*"

		with block_signals(self.ui.selectSignal) as selectSignal:
			selectSignal.addItem(name)
			selectSignal.setCurrentIndex(index)

		self.scratch_signals.append(
			Signal.EditorSignal(basic=Signal.BasicSection(name=name, type=InputType.KEY, key=32)))
		self.enable_signal_page()
		self.select_signal()
		self.append_signal_to_mapping_combos(name)

	def delete_signal(self):
		index = self.ui.selectSignal.currentIndex()
		name = self.ui.selectSignal.currentText()
		self.scratch_signals.pop(index)
		with block_signals(self.ui.selectSignal) as selectSignal:
			selectSignal.removeItem(index)
		if self.ui.selectSignal.count() == 0:
			self.disable_signal_page()
			self.mapping_clear_signals()
		else:
			self.convert_signal_to_ui(self.scratch_signals[self.ui.selectSignal.currentIndex()])
			self.remove_signal_from_mapping_combos(name)

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
		with block_signals(self.ui.signalName) as signalName:
			signalName.setText(self.ui.selectSignal.currentText())
			self.scratch_signals[self.ui.selectSignal.currentIndex()].basic.name = signalName.text()
		if self.last_signal_index >= 0:
			self.scratch_signals[self.last_signal_index] = self.convert_signal_from_ui()
		self.last_signal_index = self.ui.selectSignal.currentIndex()
		self.convert_signal_to_ui(self.scratch_signals[self.last_signal_index])

	def signal_name_changed(self):
		name = self.ui.signalName.text()
		while self.signal_exists(name):
			name = f"{name}*"

		previous_name = self.ui.selectSignal.currentText()
		index = self.ui.selectSignal.currentIndex()
		if index >= 0:
			self.ui.selectSignal.setItemText(index, name)
		with block_signals(self.ui.signalName) as signalName:
			signalName.setText(name)
		if index >= 0:
			self.scratch_signals[self.ui.selectSignal.currentIndex()].basic.name = name
		self.rename_signal_in_mapping_combos(previous_name, name)

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
		with block_signals(self.ui.signalName) as signalName:
			signalName.setText(signal.basic.name)
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

	def mapping_exists(self, mapping):
		for i in range(self.ui.selectMapping.count()):
			if i != self.ui.selectMapping.currentIndex():
				if mapping == self.ui.selectMapping.itemText(i):
					return True
		return False

	def new_mapping(self):
		index = self.ui.selectMapping.count()
		name = f"New Mapping ({index})"
		while self.mapping_exists(name):
			name = f"{name}*"

		with block_signals(self.ui.selectMapping) as selectMapping:
			selectMapping.addItem(name)
			selectMapping.setCurrentIndex(index)

		self.scratch_mappings.append(Signal.EditorMapping(name=name))
		self.enable_mapping_page()
		self.select_mapping()

	def delete_mapping(self):
		index = self.ui.selectMapping.currentIndex()
		self.scratch_mappings.pop(index)
		with block_signals(self.ui.selectMapping) as selectMapping:
			selectMapping.removeItem(index)
		if self.ui.selectMapping.count() == 0:
			self.disable_mapping_page()
		else:
			self.convert_mapping_to_ui(self.scratch_mappings[self.ui.selectMapping.currentIndex()])

	def enable_mapping_page(self):
		self.ui.selectMapping.setDisabled(False)
		self.ui.deleteMapping.setDisabled(False)
		self.ui.mappingInfoGroupBox.setDisabled(False)

	def disable_mapping_page(self):
		self.convert_mapping_to_ui(Signal.EditorMapping(name=""))
		self.ui.selectMapping.setDisabled(True)
		self.ui.deleteMapping.setDisabled(True)
		self.ui.mappingInfoGroupBox.setDisabled(True)

	def select_mapping(self):
		with block_signals(self.ui.mappingName) as mappingName:
			mappingName.setText(self.ui.selectMapping.currentText())
			self.scratch_mappings[self.ui.selectMapping.currentIndex()].name = mappingName.text()
		if self.last_mapping_index >= 0:
			self.scratch_mappings[self.last_mapping_index] = self.convert_mapping_from_ui()
		self.last_mapping_index = self.ui.selectMapping.currentIndex()
		self.convert_mapping_to_ui(self.scratch_mappings[self.last_mapping_index])

	def mapping_name_changed(self):
		name = self.ui.mappingName.text()
		while self.mapping_exists(name):
			name = f"{name}*"

		index = self.ui.selectMapping.currentIndex()
		if index >= 0:
			self.ui.selectMapping.setItemText(index, name)
		with block_signals(self.ui.mappingName) as mappingName:
			mappingName.setText(name)
		if index >= 0:
			self.scratch_mappings[index].name = name

	def convert_mapping_from_ui(self) -> Signal.EditorMapping:
		mapping = Signal.EditorMapping(name=self.ui.mappingName.text())
		for i in range(self.ui.mappingSignalTable.rowCount()):
			combo = self.ui.mappingSignalTable.cellWidget(i, 0)
			assert isinstance(combo, QComboBox)
			mapping.signals.append(combo.currentText())
		return mapping

	def convert_mapping_to_ui(self, mapping: Signal.EditorMapping):
		with block_signals(self.ui.mappingName) as mappingName:
			mappingName.setText(mapping.name)
		self.mapping_name_changed()
		for _ in range(self.ui.mappingSignalTable.rowCount()):
			self.ui.mappingSignalTable.removeRow(0)
		for i in range(len(mapping.signals)):
			self.mapping_append_signal()
			combo = self.ui.mappingSignalTable.cellWidget(i, 0)
			assert isinstance(combo, QComboBox)
			combo.setCurrentText(mapping.signals[i])

	def mapping_append_signal(self):
		row = self.ui.mappingSignalTable.rowCount()
		if row >= len(self.scratch_signals):
			QMessageBox.information(self, 'Unable to complete action', 'There are no more signals in list')
			return

		combo = QComboBox()
		unused_signals = self.get_mapping_unused_signals()
		assert len(unused_signals) > 0
		for signal in unused_signals:
			combo.addItem(signal)
		combo.setCurrentIndex(0)
		combo.currentIndexChanged.connect(self.mapping_signal_combo_changed)

		delete = QPushButton(QIcon('res/images/Delete.png'), "")
		delete.clicked.connect(lambda: self.remove_mapping_signal(row))
		delete.clicked.connect(lambda: self.set_asterisk(True))

		self.ui.mappingSignalTable.insertRow(row)
		self.ui.mappingSignalTable.setCellWidget(row, 0, combo)
		self.ui.mappingSignalTable.setCellWidget(row, 1, delete)
		self.scratch_mappings[self.ui.selectMapping.currentIndex()].signals.append(combo.currentText())
		self.mapping_signal_combo_changed()

	def mapping_clear_signals(self):
		self.scratch_mappings[self.ui.selectMapping.currentIndex()].signals.clear()
		for _ in range(self.ui.mappingSignalTable.rowCount()):
			self.ui.mappingSignalTable.removeRow(0)

	def remove_mapping_signal(self, row):
		self.scratch_mappings[self.ui.selectMapping.currentIndex()].signals.pop(row)
		self.ui.mappingSignalTable.removeRow(row)
		for i in range(row, self.ui.mappingSignalTable.rowCount()):
			delete = self.ui.mappingSignalTable.cellWidget(i, 1)
			assert isinstance(delete, QPushButton)
			delete.clicked.disconnect()
			delete.clicked.connect(lambda: self.remove_mapping_signal(i))
			delete.clicked.connect(lambda: self.set_asterisk(True))
		self.mapping_signal_combo_changed()

	def get_signal_list(self):
		signals = []
		for i in range(self.ui.selectSignal.count()):
			signals.append(self.ui.selectSignal.itemText(i))
		return signals

	def get_mapping_unused_signals(self):
		signals = self.get_signal_list()
		i = 0
		while i < len(signals):
			remove = False
			for row in range(self.ui.mappingSignalTable.rowCount()):
				combo = self.ui.mappingSignalTable.cellWidget(row, 0)
				assert isinstance(combo, QComboBox)
				if combo.currentText() == signals[i]:
					remove = True
					break
			if remove:
				signals.pop(i)
			else:
				i += 1
		return signals

	def mapping_signal_combo_changed(self):
		signal_list = self.get_signal_list()
		ordering = {signal: i for i, signal in enumerate(signal_list)}
		unused_signals = self.get_mapping_unused_signals()
		for row in range(self.ui.mappingSignalTable.rowCount()):
			combo = self.ui.mappingSignalTable.cellWidget(row, 0)
			assert isinstance(combo, QComboBox)
			with block_signals(combo):
				current_signal = combo.currentText()
				combo.clear()
				for signal in unused_signals:
					combo.addItem(signal)

				inserted = False
				for i, signal in enumerate(unused_signals):
					if ordering[current_signal] < ordering[signal]:
						combo.insertItem(i, current_signal)
						combo.setCurrentIndex(i)
						inserted = True
						break
				if not inserted:
					combo.addItem(current_signal)
					combo.setCurrentIndex(len(unused_signals))

	def append_signal_to_mapping_combos(self, signal):
		for row in range(self.ui.mappingSignalTable.rowCount()):
			combo = self.ui.mappingSignalTable.cellWidget(row, 0)
			assert isinstance(combo, QComboBox)
			with block_signals(combo):
				combo.addItem(signal)

	def remove_signal_from_mapping_combos(self, signal):
		for row in range(self.ui.mappingSignalTable.rowCount()):
			combo = self.ui.mappingSignalTable.cellWidget(row, 0)
			assert isinstance(combo, QComboBox)
			with block_signals(combo):
				if combo.currentText() == signal:
					self.remove_mapping_signal(row)
					break
				else:
					for i in range(combo.count()):
						if combo.itemText(i) == signal:
							combo.removeItem(i)
							break

		for mapping in self.scratch_mappings:
			for i in range(len(mapping.signals)):
				if mapping.signals[i] == signal:
					mapping.signals.pop(i)
					break

	def rename_signal_in_mapping_combos(self, previous_signal, new_signal):
		for row in range(self.ui.mappingSignalTable.rowCount()):
			combo = self.ui.mappingSignalTable.cellWidget(row, 0)
			assert isinstance(combo, QComboBox)
			with block_signals(combo):
				if combo.currentText() == previous_signal:
					combo.setItemText(combo.currentIndex(), new_signal)
					break
				else:
					for i in range(combo.count()):
						if combo.itemText(i) == previous_signal:
							combo.setItemText(i, new_signal)
							break

		for mapping in self.scratch_mappings:
			for i in range(len(mapping.signals)):
				if mapping.signals[i] == previous_signal:
					mapping.signals[i] = new_signal
					break
