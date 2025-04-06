#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb/stb_image.h>

#include <concepts>

namespace oly
{
	extern void init();
	extern void load_context();
	extern int terminate();

	template<typename T>
	concept numeric = std::integral<T> || std::floating_point<T>;
	class TimeImpl
	{
		double _now;
		double _delta;

	public:
		template<numeric T>
		T now() const { return (T)_now; }
		template<numeric T>
		T delta() const { return (T)_delta; }

		// LATER make these private and make Universe a friend so it can call these.
		void init() { _now = glfwGetTime(); _delta = 0.0; }
		void sync() { double n = glfwGetTime(); _delta = n - _now; _now = n; }
	};
	inline TimeImpl TIME;
}
