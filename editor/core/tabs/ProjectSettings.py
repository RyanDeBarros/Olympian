from pathlib import Path
from typing import override

import send2trash
from PySide6.QtCore import QSize
from PySide6.QtWidgets import QMessageBox, QGridLayout, QLabel, QLineEdit, QHBoxLayout, QSpacerItem, QSizePolicy, \
	QWidget

from editor import ui, TOMLAdapter
from editor.core import MainWindow, AbstractPathItem, nice_icon, block_signals, GLFW_ANY_POSITION, GLFW_DONT_CARE
from editor.core.common import SettingsForm, SettingsParameter
from .EditorTab import EditorTab
from ..CollapsibleBox import CollapsibleBox


class ProjectSettingsTab(EditorTab):
	class UID:
		def __eq__(self, other):
			return isinstance(other, ProjectSettingsTab.UID)

	def __init__(self, win: MainWindow):
		super().__init__(win)

		self.settings = {}
		self.context = {}

		self.ui = ui.ProjectSettings.Ui_ProjectSettings()
		self.ui.setupUi(self)

		self.window_settings = {}
		self.window_form = SettingsForm([
			SettingsParameter('width', self.ui.windowWidth),
			SettingsParameter('height', self.ui.windowHeight),
			SettingsParameter('title', self.ui.windowTitle)
		])

		self.window_hint_settings = {}
		self.ui.windowHintsBox = CollapsibleBox.convert_group_box(self.ui.windowHintsBox)
		self.ui.windowHintSwapIntervalAdaptive.checkStateChanged.connect(self.window_hint_swap_interval_adaptive_toggled)
		self.ui.windowHintPositionXAny.checkStateChanged.connect(self.window_hint_position_x_any_toggled)
		self.ui.windowHintPositionYAny.checkStateChanged.connect(self.window_hint_position_y_any_toggled)
		self.ui.windowHintRefreshRateDontCare.checkStateChanged.connect(self.window_hint_refresh_rate_dont_care_toggled)
		self.window_hint_form = SettingsForm([
			SettingsParameter('resizable', self.ui.windowHintResizable),
			SettingsParameter('visible', self.ui.windowHintVisible),
			SettingsParameter('decorated', self.ui.windowHintDecorated),
			SettingsParameter('focused', self.ui.windowHintFocused),
			SettingsParameter('auto_iconify', self.ui.windowHintAutoIconify),
			SettingsParameter('floating', self.ui.windowHintFloating),
			SettingsParameter('maximized', self.ui.windowHintMaximized),
			SettingsParameter('transparent_framebuffer', self.ui.windowHintTransparentFramebuffer),
			SettingsParameter('focus_on_show', self.ui.windowHintFocusOnShow),
			SettingsParameter('scale_to_monitor', self.ui.windowHintScaleToMonitor),
			SettingsParameter('scale_framebuffer', self.ui.windowHintScaleFramebuffer),
			SettingsParameter('mouse_passthrough', self.ui.windowHintMousePassthrough),
			SettingsParameter('stereo', self.ui.windowHintStereo),
			SettingsParameter('srgb_capable', self.ui.windowHintSRGBCapable),
			SettingsParameter('double_buffer', self.ui.windowHintDoubleBuffer),
			SettingsParameter('opengl_forward_compat', self.ui.windowHintOpenGLForwardCompat),
			SettingsParameter('context_debug', self.ui.windowHintContextDebug)
		])

		self.viewport_settings = {}
		self.viewport_form = SettingsForm([
			SettingsParameter('boxed', self.ui.viewportBoxed),
			SettingsParameter('stretch', self.ui.viewportStretch)
		])

		self.logger_settings = {}
		self.logger_form = SettingsForm([
			SettingsParameter('logfile', self.ui.logfile),
			SettingsParameter('append', self.ui.appendLogfile),
			SettingsParameter('console', self.ui.logToConsole)
		])

		self.ui.loggerClearLog.clicked.connect(self.clear_logger_log)

		self.ui.loggerStreamsBox = CollapsibleBox.convert_group_box(self.ui.loggerStreamsBox)

		# TODO v4 nested SettingsForms
		self.logger_enable_settings = {}
		self.logger_enable_form = SettingsForm([
			SettingsParameter('debug', self.ui.logStreamDebugEnable),
			SettingsParameter('info', self.ui.logStreamInfoEnable),
			SettingsParameter('warning', self.ui.logStreamWarningEnable),
			SettingsParameter('error', self.ui.logStreamErrorEnable),
			SettingsParameter('fatal', self.ui.logStreamFatalEnable),
		])

		self.framerate_settings = {}
		self.framerate_form = SettingsForm([
			SettingsParameter('frame_length_clip', self.ui.framerateClip),
			SettingsParameter('time_scale', self.ui.framerateTimeScale)
		])

		self.collision_settings = {}
		self.ui.collisionMasksBox = CollapsibleBox.convert_group_box(self.ui.collisionMasksBox)
		self.collision_mask_name_edits = self.init_collision_name_list(self.ui.collisionMasksBox, "Mask")
		self.ui.collisionLayersBox = CollapsibleBox.convert_group_box(self.ui.collisionLayersBox)
		self.collision_layer_name_edits = self.init_collision_name_list(self.ui.collisionLayersBox, "Layer")

		self.revert_changes_impl()
		self.window_form.connect_modified(lambda: self.set_asterisk(True))
		for control in [
			self.ui.windowHintClearColorR,
			self.ui.windowHintClearColorG,
			self.ui.windowHintClearColorB,
			self.ui.windowHintClearColorA,
			self.ui.windowHintSwapInterval,
			self.ui.windowHintSwapIntervalAdaptive,
			self.ui.windowHintPositionX,
			self.ui.windowHintPositionXAny,
			self.ui.windowHintPositionY,
			self.ui.windowHintPositionYAny,
			self.ui.windowHintRefreshRate,
			self.ui.windowHintRefreshRateDontCare
		]:
			SettingsForm.modified_event(control).connect(lambda: self.set_asterisk(True))
		self.viewport_form.connect_modified(lambda: self.set_asterisk(True))
		self.window_hint_form.connect_modified(lambda: self.set_asterisk(True))
		self.logger_form.connect_modified(lambda: self.set_asterisk(True))
		# TODO v4 with nested SettingsForm, logger_enable_form should already be connected due to logger_form being connected.
		self.logger_enable_form.connect_modified(lambda: self.set_asterisk(True))
		self.framerate_form.connect_modified(lambda: self.set_asterisk(True))

	@override
	def uid(self):
		return ProjectSettingsTab.UID()

	@override
	def icon(self, size: QSize):
		return nice_icon("res/images/Gear.png", size)

	@override
	def name(self):
		return "Project Settings"

	@override
	def save_changes_impl(self):
		# window
		self.window_settings.update(self.window_form.get_dict())

		# window hints
		self.window_hint_settings.update(self.window_hint_form.get_dict())
		self.window_hint_settings['clear_color'] = [
			self.ui.windowHintClearColorR.value(),
			self.ui.windowHintClearColorG.value(),
			self.ui.windowHintClearColorB.value(),
			self.ui.windowHintClearColorA.value()
		]
		self.window_hint_settings['swap_interval'] = self.ui.windowHintSwapInterval.value() if not self.ui.windowHintSwapIntervalAdaptive.isChecked() else -1
		self.window_hint_settings['position_x'] = self.ui.windowHintPositionX.value() if not self.ui.windowHintPositionXAny.isChecked() else GLFW_ANY_POSITION
		self.window_hint_settings['position_y'] = self.ui.windowHintPositionY.value() if not self.ui.windowHintPositionYAny.isChecked() else GLFW_ANY_POSITION
		self.window_hint_settings['refresh_rate'] = self.ui.windowHintRefreshRate.value() if not self.ui.windowHintRefreshRateDontCare.isChecked() else GLFW_DONT_CARE

		# viewport
		self.viewport_settings.update(self.viewport_form.get_dict())

		# logger
		self.logger_settings.update(self.logger_form.get_dict())
		self.logger_enable_settings.update(self.logger_enable_form.get_dict())

		# framerate
		self.framerate_settings.update(self.framerate_form.get_dict())

		# collision
		self.collision_settings['masks'] = self.get_collision_names(self.collision_mask_name_edits)
		self.collision_settings['layers'] = self.get_collision_names(self.collision_layer_name_edits)

		# dump
		TOMLAdapter.dump(self.win.project_context.project_file, self.settings)

	@override
	def revert_changes_impl(self):
		# load
		self.settings = TOMLAdapter.load(self.win.project_context.project_file)

		# context
		if 'context' not in self.settings:
			self.settings['context'] = {}
		self.context = self.settings['context']

		# window
		if 'window' not in self.context:
			self.context['window'] = {}
		self.window_settings = self.context['window']
		self.window_form.load_dict(self.window_settings)

		# window hints
		if 'window_hint' not in self.window_settings:
			self.window_settings['window_hint'] = {}
		self.window_hint_settings = self.window_settings['window_hint']
		self.window_hint_form.load_dict(self.window_hint_settings)

		clear_color = self.window_hint_settings.get('clear_color', [0.0, 0.0, 0.0, 1.0])
		self.ui.windowHintClearColorR.setValue(clear_color[0])
		self.ui.windowHintClearColorG.setValue(clear_color[1])
		self.ui.windowHintClearColorB.setValue(clear_color[2])
		self.ui.windowHintClearColorA.setValue(clear_color[3])

		swap_interval = self.window_hint_settings.get('swap_interval', 1)
		self.ui.windowHintSwapInterval.setValue(swap_interval if swap_interval >= 0 else 1)
		with block_signals(self.ui.windowHintSwapIntervalAdaptive) as windowHintSwapIntervalAdaptive:
			windowHintSwapIntervalAdaptive.setChecked(swap_interval == -1)
		self.window_hint_swap_interval_adaptive_toggled()

		position_x = self.window_hint_settings.get('position_x', GLFW_ANY_POSITION)
		self.ui.windowHintPositionX.setValue(position_x if position_x != GLFW_ANY_POSITION else 0)
		with block_signals(self.ui.windowHintPositionXAny) as windowHintPositionXAny:
			windowHintPositionXAny.setChecked(position_x == GLFW_ANY_POSITION)
		self.window_hint_position_x_any_toggled()

		position_y = self.window_hint_settings.get('position_y', GLFW_ANY_POSITION)
		self.ui.windowHintPositionY.setValue(position_y if position_y != GLFW_ANY_POSITION else 0)
		with block_signals(self.ui.windowHintPositionYAny) as windowHintPositionYAny:
			windowHintPositionYAny.setChecked(position_y == GLFW_ANY_POSITION)
		self.window_hint_position_y_any_toggled()

		refresh_rate = self.window_hint_settings.get('refresh_rate', GLFW_DONT_CARE)
		self.ui.windowHintRefreshRate.setValue(refresh_rate if refresh_rate != GLFW_DONT_CARE else 0)
		with block_signals(self.ui.windowHintRefreshRateDontCare) as windowHintRefreshRateDontCare:
			windowHintRefreshRateDontCare.setChecked(refresh_rate == GLFW_DONT_CARE)
		self.window_hint_refresh_rate_dont_care_toggled()

		# viewport
		if 'viewport' not in self.window_settings:
			self.window_settings['viewport'] = {}
		self.viewport_settings = self.window_settings['viewport']
		self.viewport_form.load_dict(self.viewport_settings)

		# logger
		if 'logger' not in self.context:
			self.context['logger'] = {}
		self.logger_settings = self.context['logger']
		self.logger_form.load_dict(self.logger_settings)

		if 'enable' not in self.logger_settings:
			self.logger_settings['enable'] = {}
		self.logger_enable_settings = self.logger_settings['enable']
		self.logger_enable_form.load_dict(self.logger_enable_settings)

		# framerate
		if 'framerate' not in self.context:
			self.context['framerate'] = {}
		self.framerate_settings = self.context['framerate']
		self.framerate_form.load_dict(self.framerate_settings)

		# collision
		if 'collision' not in self.context:
			self.context['collision'] = {}
		self.collision_settings = self.context['collision']
		self.load_collision_names(self.collision_settings.get('masks', []), self.collision_mask_name_edits)
		self.load_collision_names(self.collision_settings.get('layers', []), self.collision_layer_name_edits)

	@override
	def rename_impl(self, item: AbstractPathItem):
		raise RuntimeError("Project settings tab cannot be renamed")

	@override
	def refresh_impl(self):
		pass

	def window_hint_swap_interval_adaptive_toggled(self):
		self.ui.windowHintSwapInterval.setDisabled(self.ui.windowHintSwapIntervalAdaptive.isChecked())

	def window_hint_position_x_any_toggled(self):
		self.ui.windowHintPositionX.setDisabled(self.ui.windowHintPositionXAny.isChecked())

	def window_hint_position_y_any_toggled(self):
		self.ui.windowHintPositionY.setDisabled(self.ui.windowHintPositionYAny.isChecked())

	def window_hint_refresh_rate_dont_care_toggled(self):
		self.ui.windowHintRefreshRate.setDisabled(self.ui.windowHintRefreshRateDontCare.isChecked())

	def init_collision_name_list(self, box: CollapsibleBox, label_prefix: str) -> list[QLineEdit]:
		grid_widget = QWidget()
		grid = QGridLayout(grid_widget)
		rows = 8
		cols = 4

		name_edits = []
		for row in range(rows):
			for col in range(cols):
				layout = QHBoxLayout()
				spacer = QSpacerItem(40, 20, QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Minimum)
				layout.addItem(spacer)
				layout.addWidget(QLabel(f"{label_prefix} {row * cols + col}"))
				name_edit = QLineEdit()
				name_edits.append(name_edit)
				layout.addWidget(name_edit)
				layout.addItem(spacer)
				grid.addLayout(layout, row, col)

		for name_edit in name_edits:
			# noinspection PyShadowingNames
			def callback(name_edit=name_edit):
				name = name_edit.text()
				name_edit.clearFocus()
				if name == "":
					return
				names = self.get_collision_names(name_edits)
				index = name_edits.index(name_edit)

				def name_exists(nm):
					# noinspection PyShadowingNames
					for i in range(len(names)):
						if i != index and names[i] == nm:
							return True
					return False

				base_name = name
				i = 1
				while name_exists(name):
					name = f"{base_name} ({i})"
					i += 1
				with block_signals(name_edit):
					name_edit.setText(name)

			name_edit.editingFinished.connect(callback)
			name_edit.textChanged.connect(lambda: self.set_asterisk(True))

		box.ui.contentArea.layout().addWidget(grid_widget)
		return name_edits

	@staticmethod
	def load_collision_names(names: list[str], name_edits: list[QLineEdit]):
		seen_names = []
		for i in range(len(name_edits)):
			with block_signals(name_edits[i]) as name_edit:
				name_edit.setText(names[i] if i < len(names) and names[i] not in seen_names else "")
				seen_names.append(name_edit.text())

	@staticmethod
	def get_collision_names(name_edits: list[QLineEdit]) -> list[str]:
		names = []
		for name_edit in name_edits:
			names.append(name_edit.text())
		return names

	def clear_logger_log(self):
		logfile = Path(self.ui.logfile.text())
		if logfile.exists():
			reply = QMessageBox.question(self, "Confirm Action", "Are you sure you want to clear the project log?",
										 defaultButton=QMessageBox.StandardButton.No)
			if reply == QMessageBox.StandardButton.Yes:
				send2trash.send2trash(logfile)
