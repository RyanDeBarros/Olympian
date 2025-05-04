#pragma once

#include "external/GL.h"

namespace oly
{
	extern void enable_color();
	extern void disable_color();

	namespace stencil
	{
		extern void begin();
		extern void end();
		extern void enable_drawing(GLuint mask = 0xFF);
		extern void disable_drawing();
		
		namespace draw
		{
			extern void replace();
			extern void add();
		}
		
		namespace crop
		{
			extern void match();
			extern void miss();
		}
	}
}
