#include "ProjectContext.h"

#ifndef OLYMPIAN_CONTEXT_PROJECT_FILE
#error "OLYMPIAN_CONTEXT_PROJECT_FILE macro is not defined! Did you forget to configure CMake?"
#endif

#ifndef OLYMPIAN_CONTEXT_PROJECT_RESOURCE_DIR
#error "OLYMPIAN_CONTEXT_PROJECT_RESOURCE_DIR macro is not defined! Did you forget to configure CMake?"
#endif

namespace oly
{
	ProjectContext::ProjectContext() : context(OLYMPIAN_CONTEXT_PROJECT_FILE, OLYMPIAN_CONTEXT_PROJECT_RESOURCE_DIR) {}
}
