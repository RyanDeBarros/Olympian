from contextlib import contextmanager
from pathlib import Path
from typing import TypeVar, Generator

from PySide6.QtCore import QSize
from PySide6.QtGui import QIcon, QPixmap, Qt, QPainter
from PySide6.QtWidgets import QWidget

T = TypeVar("T", bound=QWidget)


@contextmanager
def block_signals(widget: T) -> Generator[T, None, None]:
	block_initially = widget.blockSignals(True)
	try:
		yield widget
	finally:
		widget.blockSignals(block_initially)


@contextmanager
def block_all_signals(widget: T) -> Generator[T, None, None]:
	widgets = [widget, *widget.findChildren(QWidget)]
	block_initially = [w.blockSignals(True) for w in widgets]
	try:
		yield widget
	finally:
		for w, block in zip(widgets, block_initially):
			w.blockSignals(block)


@contextmanager
def create_painter(pixmap: QPixmap) -> Generator[QPainter, None, None]:
	painter = QPainter(pixmap)
	try:
		yield painter
	finally:
		painter.end()


def nice_pixmap(pixmap: QPixmap, size: QSize) -> QPixmap | None:
	if pixmap.isNull():
		return None
	if pixmap.height() == 0 or size.height() == 0:
		return None

	original_ratio = pixmap.width() / pixmap.height()
	target_ratio = size.width() / size.height()
	pixmap = pixmap.scaled(size, Qt.AspectRatioMode.KeepAspectRatio, Qt.TransformationMode.FastTransformation)
	if abs(original_ratio - target_ratio) < 0.01:
		return pixmap

	final_pixmap = QPixmap(size)
	final_pixmap.fill(Qt.GlobalColor.transparent)
	x = (size.width() - pixmap.width()) // 2
	y = (size.height() - pixmap.height()) // 2
	with create_painter(final_pixmap) as painter:
		painter.drawPixmap(x, y, pixmap)
	return final_pixmap


def nice_icon(path: Path | str, size: QSize) -> QIcon | None:
	pixmap = nice_pixmap(QPixmap(path), size)
	if pixmap is not None:
		return QIcon(pixmap)
	else:
		return None
