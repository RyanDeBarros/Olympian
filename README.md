# Olympian
Olympian Engine is a game engine written with C++ OpenGL to make future graphics projects easier, faster, and safer. It consists of three major components, which are subject to different licesnses:
* The game framework: located in `engine/`, contains source code for the runtime engine, and the general public API that is included in `engine/Olympian.h`. For specialized features, refer to the (future) web documentation for which files to include.
* The editor: located in `editor/`.
* The detailer: located in `detail/`, serves as the ground-truth for asset formats/values, ensuring the engine and editor communicate in sync.

To access the web documentation, run `mkdocs/Serve.py`.

The editor GUI is partially built off of [imgui-toolkit](https://github.com/RyanDeBarros/imgui-toolkit), a library I'm building in conjunction with the editor.

## Licensing

All third-party licenses can be found in the `licenses/` folder.

### Engine

The engine uses the following libraries:

- **nigels-com/glew**
- **GLFW**
- **g-truc/glm**
- **nothings/stb**
- **marzer/tomlplusplus**
- **memononen/nanosvg**
- **sasamil/Quartic**

### Editor

The editor uses the following libraries:

- **nigels-com/glew**
- **GLFW**
- **ocornut/imgui**
- **aiekick/ImGuiFileDialog**
- **g-truc/glm**
- **nothings/stb**
- **marzer/tomlplusplus**
- **memononen/nanosvg**

TODO v9.4 use containing folders for licenses instead of writing the library name in the filename itself. Also add a detail/ licenses folder

The editor also uses assets from the following resources:

- [Google Icons](https://fonts.google.com/icons)

### Detailer

The detail module uses the following libraries:

- **nigels-com/glew**
- **GLFW**
- **marzer/tomlplusplus**

### Documentation

The documentation webpage uses the following libraries:

- **mkdocs**
- **mkdocs-material**

### Other

Additionally, extra assets such as fonts or images may be used. For the actual licensing information, refer to the `licenses/` folder.

Note: some SIL OFL fonts are licensed separately, since they are not compatible under GNU GPL v3. Dependencies are not included directly in source code, but are referenced by CMake. All source code is written by me, and is therefore under GPLv3.
