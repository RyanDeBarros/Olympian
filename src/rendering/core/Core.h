#pragma once

#include "Shaders.h"
#include "VertexArrays.h"
#include "Textures.h"
#include "Window.h"

namespace oly
{
	namespace rendering
	{
		template<std::integral Type>
		constexpr std::array<Type, 6> quad_indices(Type quad_index)
		{
			return std::array<Type, 6>{ Type(0 + 4 * quad_index), Type(1 + 4 * quad_index), Type(2 + 4 * quad_index), Type(2 + 4 * quad_index), Type(3 + 4 * quad_index), Type(0 + 4 * quad_index) };
		}

		template<std::integral Type>
		constexpr void quad_indices(Type* indices, Type quad_index)
		{
			indices[0] = Type(0 + 4 * quad_index);
			indices[1] = Type(1 + 4 * quad_index);
			indices[2] = Type(2 + 4 * quad_index);
			indices[3] = Type(2 + 4 * quad_index);
			indices[4] = Type(3 + 4 * quad_index);
			indices[5] = Type(0 + 4 * quad_index);
		}
	}

	extern void enable_color();
	extern void disable_color();

	namespace stencil
	{
		extern void begin();
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
		extern void end();
	}
}
