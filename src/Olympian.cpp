#include "Olympian.h"

#include "util/Errors.h"
#include "rendering/Resources.h"

void oly::init()
{
	if (glfwInit() != GLFW_TRUE)
		throw oly::Error(oly::ErrorCode::GLFW_INIT);
	stbi_set_flip_vertically_on_load(true);
	LOG.set_logfile("../../../Olympian.log", true); // LATER use independent filepath
	LOG.flush();
}

void oly::load_context()
{
	load_resources();
	TIME.init();
}

int oly::terminate()
{
	unload_resources();
	glfwTerminate();
	return 0;
}

void oly::pre_frame()
{
	check_errors();
	LOG.flush();
	glfwPollEvents();
	TIME.sync();
}
