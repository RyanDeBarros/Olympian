target_sources(OlympianEditor PRIVATE
	DynamicList.cpp
	GUIState.cpp
	ImGuiWrapper.cpp
	ListModel.cpp
	PropertyClipboard.cpp
	PropertyGrid.cpp
	PropertyPayloads.cpp
	PropertyViews.cpp
	StyleStack.cpp
	WidgetComponentCommon.cpp
)

add_subdirectory(graphics)
add_subdirectory(scopes)
