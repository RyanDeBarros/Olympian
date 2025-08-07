from PySide6.QtWidgets import QApplication, QMainWindow

from editor import StartMenuWindow

if __name__ == "__main__":
	app = QApplication([])
	window = StartMenuWindow()
	window.show()
	app.exec()
