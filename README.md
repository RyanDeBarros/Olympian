# Olympian
Olympian Engine is a game engine written with C++ OpenGL to make future graphics projects easier, faster, and safer. It consists of two major components - the game framework (located in engine/), and the editor (located in editor/), which are subject to different licenses.

The engine/ folder contains source code for the framework, and the general public API is included in engine/Olympian.h. For specialized features, refer to the (future) web documentation for which files to include.

To build the editor, simply run editor/Build.py. This will build an executable, editor/OlyEditor.exe. Do not move the executable from editor/.

## Licensing

All third-party licenses can be found in the licenses/ folder.

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

- **PySide6**
- **Send2Trash**

### Other

Additionally, extra assets such as fonts or images may be used. For the actual licensing information, refer to the licenses/ folder.

Note that some SIL OFL fonts are licensed separately, since they are not compatible under GNU GPL v3. Dependencies are not included directly in source code, but are referenced by CMake. All source code is written by me, and is therefore under GPLv3.
