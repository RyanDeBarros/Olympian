from __future__ import annotations

from typing import TYPE_CHECKING

from editor.core import SettingsForm, SettingsParameter

if TYPE_CHECKING:
	from .ProjectSettings import ProjectSettingsTab


class SimulationSettingsManager:
	def __init__(self, tab: ProjectSettingsTab):
		self.tab = tab
		self.ui = self.tab.ui

		self.framerate_settings = {}
		self.framerate_form = SettingsForm([
			SettingsParameter('frame_length_clip', self.ui.framerateClip),
			SettingsParameter('time_scale', self.ui.framerateTimeScale)
		])

	def connect_modified(self):
		self.framerate_form.connect_modified(lambda: self.tab.set_asterisk(True))

	def save(self):
		self.framerate_settings.update(self.framerate_form.get_dict())

	def load(self):
		if 'framerate' not in self.tab.context:
			self.tab.context['framerate'] = {}
		self.framerate_settings = self.tab.context['framerate']
		self.framerate_form.load_dict(self.framerate_settings)
