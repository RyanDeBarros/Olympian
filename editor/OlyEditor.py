from PySide6.QtWidgets import QApplication, QMainWindow, QWidget, QStackedWidget, QDialogButtonBox

import toml

import ui


class ManifestTOML:
    def __init__(self):
        self.filepath = 'data/manifest.toml'
        with open(self.filepath, 'r') as f:
            self.toml = toml.load(f)

    def dump(self):
        with open(self.filepath, 'w') as f:
            toml.dump(self.toml, f)


class ManifestWidget(QWidget):
    def __init__(self, manifest: ManifestTOML):
        super().__init__()
        self.manifest = manifest
        self.dirty = False

        self.ui = ui.manifest.Ui_Form()
        self.ui.setupUi(self)

        self.ui.confirmationBox.button(QDialogButtonBox.StandardButton.Save).clicked.connect(self.save)
        self.ui.confirmationBox.button(QDialogButtonBox.StandardButton.Cancel).clicked.connect(self.cancel)

        self.manifest_box = self.ui.manifestBox

        self.res_input = self.ui.res
        if 'res' not in self.manifest.toml:
            self.manifest.toml['res'] = "../res"
        self.res_input.setText(self.manifest.toml['res'])
        self.res_input.textChanged.connect(self.mark_dirty)

    def save(self):
        self.manifest.toml['res'] = self.res_input.text()
        self.manifest.dump()
        self.set_dirty(False)

    def cancel(self):
        self.res_input.setText(self.manifest.toml['res'])
        self.set_dirty(False)

    def mark_dirty(self):
        self.set_dirty(True)

    def set_dirty(self, dirty):
        if self.dirty:
            if not dirty:
                self.manifest_box.setTitle("Manifest")
        elif dirty:
            self.manifest_box.setTitle("*Manifest")
        self.dirty = dirty


class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()

        self.setWindowTitle("Olympian Editor")
        # TODO v3 create and set window icon

        self.stack = QStackedWidget()
        self.setCentralWidget(self.stack)

        self.manifest_toml = ManifestTOML()
        self.manifest_screen = ManifestWidget(self.manifest_toml)

        self.stack.addWidget(self.manifest_screen)

        self.stack.setCurrentWidget(self.manifest_screen)


if __name__ == "__main__":
    app = QApplication([])
    window = MainWindow()
    window.show()
    app.exec()
