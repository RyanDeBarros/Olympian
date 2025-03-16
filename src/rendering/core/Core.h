#pragma once

#include "rendering/core/Shaders.h"
#include "rendering/core/Buffers.h"
#include "rendering/core/VertexArrays.h"
#include "rendering/core/Textures.h"

namespace oly
{
	namespace rendering
	{
		template<typename CPUData>
		struct Batch;

		template<typename CPUData>
		void attrib_layout(const Batch<CPUData>&)
		{
			static_assert(false);
		}

		template<typename CPUData>
		void draw(const Batch<CPUData>&)
		{
			static_assert(false);
		};

		template<typename CPUData>
		void draw(const Batch<CPUData>&, size_t)
		{
			static_assert(false);
		};

		template<typename CPUData>
		struct Batch
		{
			CPUData cpu_data = {};

			std::shared_ptr<VAODescriptor> vao_descriptor;
			std::shared_ptr<Shader> shader;

			Batch() = default;
			template<typename... CPUDataArgs>
			Batch(CPUDataArgs&&... args) : cpu_data(std::forward<CPUDataArgs>(args)...) {}

			GLuint get_vao() const { return vao_descriptor->vao; }
			GLuint get_vbo(GLsizei i = 0) const { return vao_descriptor->get_vbo(i); }
			GLuint get_ebo() const { return vao_descriptor->get_ebo(); }
			GLuint get_shader() const { return *shader; }

			void gen_vao_descriptor(GLsizei n, bool ebo)
			{
				vao_descriptor = std::make_shared<VAODescriptor>();
				if (n > 0)
					vao_descriptor->gen_vbos(n);
				if (ebo)
					vao_descriptor->gen_ebo();
				glBindVertexArray(vao_descriptor->vao);
				oly::rendering::attrib_layout(*this);
			}
		};
	}
}
