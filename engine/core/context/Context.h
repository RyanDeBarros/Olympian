#pragma once

#include <imp/instance_guard.hpp>

namespace oly::context
{
	class Context : public imp::instance_guard<Context>
	{
	public:
		Context();
		~Context();
	    void run();
	};

    namespace internal
    {
        extern bool render_frame();
    }
}
