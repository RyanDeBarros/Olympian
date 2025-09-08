from pathlib import Path
from typing import override

import send2trash
from PySide6.QtCore import QSize
from PySide6.QtWidgets import QMessageBox, QGridLayout, QLabel, QLineEdit, QHBoxLayout, QSpacerItem, QSizePolicy, \
	QWidget

from editor import ui, TOMLAdapter
from editor.core import MainWindow, AbstractPathItem, nice_icon, block_signals
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
		# TODO v4 window hints

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
		self.ui.collisionMasksCollapsibleBox = CollapsibleBox.convert_group_box(self.ui.collisionMasksCollapsibleBox)
		self.collision_mask_name_edits = self.init_collision_name_list(self.ui.collisionMasksCollapsibleBox, "Mask")
		self.ui.collisionLayersCollapsibleBox = CollapsibleBox.convert_group_box(self.ui.collisionLayersCollapsibleBox)
		self.collision_layer_name_edits = self.init_collision_name_list(self.ui.collisionLayersCollapsibleBox, "Layer")

		self.revert_changes_impl()
		self.window_form.connect_modified(lambda: self.set_asterisk(True))
		self.viewport_form.connect_modified(lambda: self.set_asterisk(True))
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
		self.window_settings.update(self.window_form.get_dict())
		self.viewport_settings.update(self.viewport_form.get_dict())
		self.logger_settings.update(self.logger_form.get_dict())
		self.logger_enable_settings.update(self.logger_enable_form.get_dict())
		self.framerate_settings.update(self.framerate_form.get_dict())
		self.collision_settings['masks'] = self.get_collision_names(self.collision_mask_name_edits)
		self.collision_settings['layers'] = self.get_collision_names(self.collision_layer_name_edits)
		TOMLAdapter.dump(self.win.project_context.project_file, self.settings)

	@override
	def revert_changes_impl(self):
		self.settings = TOMLAdapter.load(self.win.project_context.project_file)

		if 'context' not in self.settings:
			self.settings['context'] = {}
		self.context = self.settings['context']

		if 'window' not in self.context:
			self.context['window'] = {}
		self.window_settings = self.context['window']
		self.window_form.load_dict(self.window_settings)

		if 'viewport' not in self.window_settings:
			self.window_settings['viewport'] = {}
		self.viewport_settings = self.window_settings['viewport']
		self.viewport_form.load_dict(self.viewport_settings)

		if 'logger' not in self.context:
			self.context['logger'] = {}
		self.logger_settings = self.context['logger']
		self.logger_form.load_dict(self.logger_settings)

		if 'enable' not in self.logger_settings:
			self.logger_settings['enable'] = {}
		self.logger_enable_settings = self.logger_settings['enable']
		self.logger_enable_form.load_dict(self.logger_enable_settings)

		if 'framerate' not in self.context:
			self.context['framerate'] = {}
		self.framerate_settings = self.context['framerate']
		self.framerate_form.load_dict(self.framerate_settings)

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
