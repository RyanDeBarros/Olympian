target_sources(OlympianEditor PRIVATE
	DynamicList.cpp
	GUIState.cpp
	ImGuiWrapper.cpp
	ListModel.cpp
	UnsavedChangesModal.cpp
)

add_subdirectory(graphics)
add_subdirectory(scopes)
