target_sources(OlympianEditor PRIVATE
	Controls.cpp
	DrawResult.cpp
	DynamicList.cpp
	GUIState.cpp
	ImGuiWrapper.cpp
	InlineWidget.cpp
	ListModel.cpp
	UnsavedChangesModal.cpp
	WidgetComponentCommon.cpp
)

add_subdirectory(graphics)
add_subdirectory(properties)
add_subdirectory(scopes)
