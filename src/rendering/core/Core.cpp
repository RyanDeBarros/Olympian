#include "Core.h"

void oly::stencil::begin()
{
	glEnable(GL_STENCIL_TEST);
}

void oly::stencil::enable_drawing(GLuint mask)
{
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glStencilMask(mask);
}

void oly::stencil::disable_drawing()
{
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glStencilMask(0x00);
}

void oly::stencil::draw::replace()
{
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
}

void oly::stencil::draw::add()
{
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_INCR, GL_INCR);
}

void oly::stencil::crop::match()
{
	glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
}

void oly::stencil::crop::miss()
{
	glStencilFunc(GL_EQUAL, 0, 0xFF);
}

void oly::stencil::end()
{
	glDisable(GL_STENCIL_TEST);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}
