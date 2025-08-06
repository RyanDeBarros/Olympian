import os
import subprocess

if __name__ == "__main__":
    folder = "ui"
    if os.path.exists(folder):
        for filename in os.listdir(folder):
            if filename.endswith(".ui"):
                ui_path = os.path.join(folder, filename)
                py_filename = filename.replace(".ui", ".py")
                py_path = os.path.join(folder, py_filename)

                print(f"Converting {ui_path} -> {py_path}")
                subprocess.run(["pyside6-uic", ui_path, "-o", py_path], check=True)
