import os
import subprocess

if __name__ == "__main__":
    folder = "ui"

    def path(filename):
        return os.path.join(folder, os.path.relpath(os.path.join(root, filename), folder))

    for root, dirs, files in os.walk(folder):
        for file in files:
            if file.endswith(".ui"):
                ui_path = path(file)
                py_file = file.replace(".ui", ".py")
                py_path = path(py_file)

                print(f"Converting {ui_path} -> {py_path}")
                subprocess.run(["pyside6-uic", ui_path, "-o", py_path], check=True)
