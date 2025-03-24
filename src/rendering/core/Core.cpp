#include "Core.h"

void oly::enable_color()
{
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

void oly::disable_color()
{
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
}

void oly::stencil::begin()
{
	glEnable(GL_STENCIL_TEST);
}

void oly::stencil::enable_drawing(GLuint mask)
{
	glStencilMask(mask);
}

void oly::stencil::disable_drawing()
{
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
	enable_color();
}
