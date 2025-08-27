from contextlib import contextmanager
from typing import TypeVar, Generator

from PySide6.QtWidgets import QWidget


T = TypeVar("T", bound=QWidget)

@contextmanager
def block_signals(widget: T) -> Generator[T, None, None]:
	block_initially = widget.blockSignals(True)
	try:
		yield widget
	finally:
		widget.blockSignals(block_initially)
