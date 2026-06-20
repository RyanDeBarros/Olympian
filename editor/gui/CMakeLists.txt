target_sources(OlympianEditor PRIVATE
	DynamicList.cpp
	GUIState.cpp
	ImGuiWrapper.cpp
	InlineWidget.cpp
	ListModel.cpp
	StyleStack.cpp
	WidgetComponentCommon.cpp
)

add_subdirectory(graphics)
add_subdirectory(scopes)
