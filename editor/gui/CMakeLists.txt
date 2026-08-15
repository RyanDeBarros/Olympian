target_sources(OlympianEditor PRIVATE
	DynamicList.cpp
	ImGuiWrapper.cpp
	InputData.cpp
	ListModel.cpp
	UnsavedChangesModal.cpp
)

add_subdirectory(graphics)
add_subdirectory(scopes)
