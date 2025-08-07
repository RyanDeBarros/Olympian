#pragma once

#include "Olympian.h"

namespace oly
{
	struct ProjectContext
	{
		ProjectContext();
		ProjectContext(const ProjectContext&) = delete;
		ProjectContext(ProjectContext&&) = delete;

	private:
		context::Context context;
	};
}
