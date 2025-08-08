from PySide6.QtWidgets import QApplication

from editor import StartMenuWindow, ProjectWindow


windows = {}


def open_start_menu():
	windows['start'] = StartMenuWindow(open_project)
	windows['start'].show()


def open_project(project_filepath):
	windows['project'] = ProjectWindow(open_start_menu, project_filepath)
	windows['project'].show()


if __name__ == "__main__":
	app = QApplication([])
	open_start_menu()
	app.exec()
