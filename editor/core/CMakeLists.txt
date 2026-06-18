target_sources(OlympianEditor PRIVATE
	Errors.cpp
	InputListener.cpp
	Main.cpp
	PathInfo.cpp
	Types.cpp
)

add_subdirectory(editor)
add_subdirectory(windows)
