from PySide6.QtCore import Qt
from PySide6.QtWidgets import QPlainTextEdit, QWidget


class ZoomableTextEdit(QPlainTextEdit):
	INITIAL_FONT_SIZE = 12.0

	def init(self):
		self.set_font_size(ZoomableTextEdit.INITIAL_FONT_SIZE)

	def wheelEvent(self, event):
		if event.modifiers() & Qt.KeyboardModifier.ControlModifier:
			delta = event.angleDelta().y() / 120
			self.set_font_size(max(1, self.font().pointSizeF() + delta))
		else:
			super().wheelEvent(event)

	def set_font_size(self, sz):
		font = self.font()
		font.setPointSizeF(sz)
		self.setFont(font)
		self.setTabStopDistance(self.fontMetrics().horizontalAdvance(' ' * 4))
		ZoomableTextEdit.INITIAL_FONT_SIZE = font.pointSizeF()
