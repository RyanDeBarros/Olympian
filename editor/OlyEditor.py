from PySide6.QtWidgets import QApplication

from editor.core import StartMenuWindow, MainWindow

windows = {}

# TODO v4 use window icon for application icon


def open_start_menu():
	windows['start'] = StartMenuWindow(open_project)
	windows['start'].show()


def open_project(project_filepath):
	windows['project'] = MainWindow(open_start_menu, project_filepath)
	windows['project'].show()


if __name__ == "__main__":
	app = QApplication([])
	open_start_menu()
	app.exec()


# TODO v5 update .gitignore to exclude projects/, as that is a local-specific folder and should not be included in actual engine.
