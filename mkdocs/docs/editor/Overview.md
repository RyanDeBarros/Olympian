## Building the editor
To build the editor, run this in the command line: ```python editor/Build.py```. Note that the `PyInstaller` package must be installed. To install it, run ```pip install pyinstaller```.

## Start menu
The start menu presents 4 options:

* **New**: Create a new project. It's important to create new projects through this menu, as it will auto-generate important files.
* **Open**: Open an existing project. Select the `<project-name>.oly` project file.
* **Recent**: Open a recent project in the editor's manifest.
* **Delete**: Deletes an existing project. *Note*: this does not delete the actual files, only de-registers it from the editor's manifest.
