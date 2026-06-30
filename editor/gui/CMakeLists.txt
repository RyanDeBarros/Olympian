target_sources(OlympianEditor PRIVATE
	DrawResult.cpp
	DynamicList.cpp
	GUIState.cpp
	ImGuiWrapper.cpp
	InlineWidget.cpp
	ListModel.cpp
	StyleStack.cpp
	UnsavedChangesModal.cpp
	WidgetComponentCommon.cpp
)

add_subdirectory(graphics)
add_subdirectory(properties)
add_subdirectory(scopes)
