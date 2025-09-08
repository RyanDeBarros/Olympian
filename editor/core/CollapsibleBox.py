from PySide6.QtGui import Qt
from PySide6.QtWidgets import QFrame

from editor import ui
from editor.core import block_signals


class CollapsibleBox(QFrame):
	def __init__(self, title="", expanded=False, parent=None):
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
