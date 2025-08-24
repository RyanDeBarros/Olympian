from PySide6.QtWidgets import QMessageBox


def alert_error(parent, title, desc):
	msg = QMessageBox(parent)
	msg.setIcon(QMessageBox.Icon.Warning)
	msg.setWindowTitle(title)
	msg.setText(desc)
	msg.exec()
