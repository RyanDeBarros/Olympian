from __future__ import annotations

from typing import TYPE_CHECKING

from PySide6.QtWidgets import QLineEdit, QHBoxLayout, QSpacerItem, QSizePolicy, QLabel, QWidget, QGridLayout

from editor.core import block_signals
from ...CollapsibleBox import CollapsibleBox

if TYPE_CHECKING:
	from .ProjectSettings import ProjectSettingsTab


class PhysicsSettingsManager:
	def __init__(self, tab: ProjectSettingsTab):
		self.tab = tab
		self.ui = self.tab.ui

		self.collision_settings = {}
		self.ui.collisionMasksBox = CollapsibleBox.convert_group_box(self.ui.collisionMasksBox)
		self.collision_mask_name_edits = self.init_collision_name_list(self.ui.collisionMasksBox, "Mask")
		self.ui.collisionLayersBox = CollapsibleBox.convert_group_box(self.ui.collisionLayersBox)
		self.collision_layer_name_edits = self.init_collision_name_list(self.ui.collisionLayersBox, "Layer")

	@staticmethod
	def init_collision_name_list(box: CollapsibleBox, label_prefix: str) -> list[QLineEdit]:
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

		box.ui.contentArea.layout().addWidget(grid_widget)
		return name_edits

	def connect_collision_name_list_modified(self, name_edits: list[QLineEdit]):
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
			name_edit.textChanged.connect(lambda: self.tab.set_asterisk(True))

	def connect_modified(self):
		self.connect_collision_name_list_modified(self.collision_mask_name_edits)
		self.connect_collision_name_list_modified(self.collision_layer_name_edits)

	def save(self):
		self.collision_settings['masks'] = self.get_collision_names(self.collision_mask_name_edits)
		self.collision_settings['layers'] = self.get_collision_names(self.collision_layer_name_edits)

	def load(self):
		if 'collision' not in self.tab.context:
			self.tab.context['collision'] = {}
		self.collision_settings = self.tab.context['collision']
		self.load_collision_names(self.collision_settings.get('masks', []), self.collision_mask_name_edits)
		self.load_collision_names(self.collision_settings.get('layers', []), self.collision_layer_name_edits)

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
