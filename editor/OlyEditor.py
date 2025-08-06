from PySide6.QtWidgets import QApplication, QMainWindow

from editor import ManifestTOML
from editor import StartMenuWidget


class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()

        self.setWindowTitle("Olympian Editor")
        # TODO v3 create and set window icon

        self.manifest_toml = ManifestTOML()
        self.setCentralWidget(StartMenuWidget(self.manifest_toml))


if __name__ == "__main__":
    app = QApplication([])
    window = MainWindow()
    window.show()
    app.exec()
