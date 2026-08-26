target_sources(OlympianEditor PRIVATE
	SpecialUndoActions.cpp
	InputListener.cpp
	MemoryUnit.cpp
	PathInfo.cpp
	Types.cpp
)

add_subdirectory(editor)
add_subdirectory(windows)
