Welcome to the docs! Just like Olympian, the documentation is divided into 2 main components - the engine and the editor.

## Project layout
An Olympian project has the following structure, which is auto-generated when creating a new project in the editor:

```
ProjectName/
├── src/                        (game source code)
│   ├── ProjectContext.h        (auto-generated file)
│   ├── ProjectContext.cpp      (auto-generated file)
│   └── ProjectName.cpp         (program entry point)
├── res/                        (game assets)
├── .gen/                       (auto-generated code)
│   ├── archetypes/
│   ├── manifest.txt            (list of folders in res/ to search for archetypes)
│   └── cache.json
├── .settings/                  (internal settings used by editor)
├── .trash/                     (internal trash bin)
├── .gitignore
├── CMakeLists.txt
├── ProjectName.log
└── ProjectName.oly
```

The public game folders are `src/` (for source code) and `res/` (for assets/resources). Everything else should be interacted with through the editor.
