#include "Olympian.h"

#include "util/Errors.h"
#include "rendering/Resources.h"

void oly::init()
{
	if (glfwInit() != GLFW_TRUE)
		throw oly::Error(oly::ErrorCode::GLFW_INIT);
	stbi_set_flip_vertically_on_load(true);
	LOG.set_logfile("../../../Olympian.log", true); // LATER use independent filepath
}

void oly::load_context()
{
	load_resources();
}

int oly::terminate()
{
	unload_resources();
	glfwTerminate();
	return 0;
}
