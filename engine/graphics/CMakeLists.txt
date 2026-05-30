target_sources(OlympianEngine PUBLIC
	Camera.cpp
	Drawable.cpp
	Stencil.cpp
)

add_subdirectory(backend)
add_subdirectory(particles)
add_subdirectory(resources)
add_subdirectory(shapes)
add_subdirectory(sprites)
add_subdirectory(text)
