from PySide6.QtGui import Qt
from PySide6.QtWidgets import QFrame, QWidget, QGroupBox

from editor import ui
from editor.core import block_signals


class CollapsibleBox(QFrame):
	def __init__(self, title: str = "", expanded: bool = False, parent: QWidget | None = None):
		super().__init__(parent)

		self.ui = ui.CollapsibleBox.Ui_CollapsibleBox()
		self.ui.setupUi(self)
		self.set_title(title)
		self.ui.toggleButton.clicked.connect(self.handle_toggle)
		with block_signals(self.ui.toggleButton) as toggleButton:
			toggleButton.setChecked(expanded)
		self.handle_toggle()

	def handle_toggle(self):
		expanded = self.ui.toggleButton.isChecked()
		self.ui.contentArea.setVisible(expanded)
		self.ui.toggleButton.setArrowType(Qt.ArrowType.DownArrow if expanded else Qt.ArrowType.RightArrow)

	def set_title(self, title: str):
		self.ui.toggleButton.setText(f" {title}")

	def set_expanded(self, expanded: bool):
		self.ui.toggleButton.setChecked(expanded)

	@staticmethod
	def convert_group_box(group_box: QGroupBox, expanded: bool = False):
		parent = group_box.parentWidget()
		if parent is None:
			raise RuntimeError("GroupBox must have a parent layout")

		collapsible = CollapsibleBox(title=group_box.title(), expanded=expanded, parent=parent)
		collapsible.setObjectName(group_box.objectName())

		if (old_layout := group_box.layout()) is not None:
			target_layout = collapsible.ui.contentArea.layout()
			for i in range(old_layout.count()):
				li = old_layout.takeAt(i)
				if widget := li.widget():
					widget.setParent(collapsible.ui.contentArea)
					target_layout.addWidget(widget)
				elif layout := li.layout():
					container = QWidget()
					container.setLayout(layout)
					container.setParent(collapsible.ui.contentArea)
					target_layout.addWidget(container)
					target_layout.addChildLayout(layout)
				elif item := li.spacerItem():
					target_layout.addItem(item)
			# noinspection PyTypeChecker
			group_box.setLayout(None)
		else:
			for child in group_box.findChildren(QWidget, options=Qt.FindChildOption.FindDirectChildrenOnly):
				if child.parent() == group_box:
					child.setParent(collapsible.ui.contentArea)
					collapsible.ui.contentArea.layout().addWidget(child)

		parent_layout = parent.layout()
		if parent_layout is not None:
			index = parent_layout.indexOf(group_box)
			# noinspection PyUnresolvedReferences
			parent_layout.insertWidget(index, collapsible)
		group_box.setParent(None)

		return collapsible
