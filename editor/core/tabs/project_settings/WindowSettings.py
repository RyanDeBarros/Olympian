from __future__ import annotations

from typing import TYPE_CHECKING

from editor.core import SettingsForm, SettingsParameter, GLFW_ANY_POSITION, GLFW_DONT_CARE, block_signals
from editor.core.CollapsibleBox import CollapsibleBox

if TYPE_CHECKING:
	from .ProjectSettings import ProjectSettingsTab


class WindowSettingsManager:
	def __init__(self, tab: ProjectSettingsTab):
		self.tab = tab
		self.ui = self.tab.ui

		self.window_settings = {}
		self.window_form = SettingsForm([
			SettingsParameter('width', self.ui.windowWidth),
			SettingsParameter('height', self.ui.windowHeight),
			SettingsParameter('title', self.ui.windowTitle)
		])

		self.viewport_settings = {}
		self.viewport_form = SettingsForm([
			SettingsParameter('boxed', self.ui.viewportBoxed),
			SettingsParameter('stretch', self.ui.viewportStretch)
		])

		self.window_hint_settings = {}
		self.ui.windowHintsBox = CollapsibleBox.convert_group_box(self.ui.windowHintsBox)
		self.ui.windowHintSwapIntervalAdaptive.checkStateChanged.connect(
			self.window_hint_swap_interval_adaptive_toggled)
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

	def connect_modified(self):
		self.window_form.connect_modified(lambda: self.tab.set_asterisk(True))
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
			SettingsForm.modified_event(control).connect(lambda: self.tab.set_asterisk(True))
		self.viewport_form.connect_modified(lambda: self.tab.set_asterisk(True))
		self.window_hint_form.connect_modified(lambda: self.tab.set_asterisk(True))

	def save(self):
		# window
		self.window_settings.update(self.window_form.get_dict())

		# viewport
		self.viewport_settings.update(self.viewport_form.get_dict())

		# window hints
		self.window_hint_settings.update(self.window_hint_form.get_dict())
		self.window_hint_settings['clear_color'] = [
			self.ui.windowHintClearColorR.value(),
			self.ui.windowHintClearColorG.value(),
			self.ui.windowHintClearColorB.value(),
			self.ui.windowHintClearColorA.value()
		]
		self.window_hint_settings[
			'swap_interval'] = self.ui.windowHintSwapInterval.value() if not self.ui.windowHintSwapIntervalAdaptive.isChecked() else -1
		self.window_hint_settings[
			'position_x'] = self.ui.windowHintPositionX.value() if not self.ui.windowHintPositionXAny.isChecked() else GLFW_ANY_POSITION
		self.window_hint_settings[
			'position_y'] = self.ui.windowHintPositionY.value() if not self.ui.windowHintPositionYAny.isChecked() else GLFW_ANY_POSITION
		self.window_hint_settings[
			'refresh_rate'] = self.ui.windowHintRefreshRate.value() if not self.ui.windowHintRefreshRateDontCare.isChecked() else GLFW_DONT_CARE

	def load(self):
		# window
		if 'window' not in self.tab.context:
			self.tab.context['window'] = {}
		self.window_settings = self.tab.context['window']
		self.window_form.load_dict(self.window_settings)

		# viewport
		if 'viewport' not in self.window_settings:
			self.window_settings['viewport'] = {}
		self.viewport_settings = self.window_settings['viewport']
		self.viewport_form.load_dict(self.viewport_settings)

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

	def window_hint_swap_interval_adaptive_toggled(self):
		self.ui.windowHintSwapInterval.setDisabled(self.ui.windowHintSwapIntervalAdaptive.isChecked())

	def window_hint_position_x_any_toggled(self):
		self.ui.windowHintPositionX.setDisabled(self.ui.windowHintPositionXAny.isChecked())

	def window_hint_position_y_any_toggled(self):
		self.ui.windowHintPositionY.setDisabled(self.ui.windowHintPositionYAny.isChecked())

	def window_hint_refresh_rate_dont_care_toggled(self):
		self.ui.windowHintRefreshRate.setDisabled(self.ui.windowHintRefreshRateDontCare.isChecked())
