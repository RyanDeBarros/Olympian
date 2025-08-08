from PySide6.QtWidgets import QWidget

from editor import ui


class FontEditorWidget(QWidget):
	def __init__(self, win):
		super().__init__()
		self.win = win

		self.ui = ui.asset_editors.Font.Ui_Form()
		self.ui.setupUi(self)
