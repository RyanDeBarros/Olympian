#include "Stencil.h"

namespace oly
{
	void enable_color()
	{
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	}

	void disable_color()
	{
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	}

	namespace stencil
	{
		void begin()
		{
			glEnable(GL_STENCIL_TEST);
		}

		void end()
		{
			glDisable(GL_STENCIL_TEST);
			enable_color();
		}

		void enable_drawing(GLuint mask)
		{
			glStencilMask(mask);
		}

		void disable_drawing()
		{
			glStencilMask(0x00);
		}

		namespace draw
		{
			void replace()
			{
				glStencilFunc(GL_ALWAYS, 1, 0xFF);
				glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
			}

			void add()
			{
				glStencilFunc(GL_ALWAYS, 1, 0xFF);
				glStencilOp(GL_KEEP, GL_INCR, GL_INCR);
			}
		}

		namespace crop
		{
			void match()
			{
				glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
				glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
			}

			void miss()
			{
				glStencilFunc(GL_EQUAL, 0, 0xFF);
				glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
			}
		}
	}
}
