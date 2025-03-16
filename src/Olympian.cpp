#include "Olympian.h"

#include "util/Errors.h"

void oly::init()
{
	if (glfwInit() != GLFW_TRUE)
		throw oly::Error(oly::ErrorCode::GLFW_INIT);
	stbi_set_flip_vertically_on_load(true);
}

int oly::terminate()
{
	glfwTerminate();
	return 0;
}
