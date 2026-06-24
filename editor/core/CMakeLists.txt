target_sources(OlympianEditor PRIVATE
	Errors.cpp
	InputListener.cpp
	Main.cpp
	MemoryUnit.cpp
	PathInfo.cpp
	Types.cpp
	UndoHistory.cpp
)

add_subdirectory(editor)
add_subdirectory(windows)
