#pragma once

#include "rendering/core/Shaders.h"
#include "rendering/core/Buffers.h"
#include "rendering/core/VertexArrays.h"

namespace oly
{
	namespace rendering
	{
		struct empty {};

		template<typename VertexData>
		void attrib_layout(const VertexData&, const std::vector<std::shared_ptr<GLBuffer>>& vbos)
		{
			static_assert(false);
		}

		template<typename VertexData, typename ElementData, typename DrawSpecification>
		struct Batch
		{
			VertexData vertex_data = {};
			ElementData element_data = {};
			DrawSpecification draw_specification = {};

			std::shared_ptr<VAODescriptor> vao_descriptor;
			std::shared_ptr<Shader> shader;

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
				init_layout();
			}
			void init_layout() const
			{
				glBindVertexArray(vao_descriptor->vao);
				oly::rendering::attrib_layout(vertex_data, vao_descriptor->vbos);
			}
		};

		template<typename VertexData, typename ElementData, typename DrawSpecification>
		void draw(const Batch<VertexData, ElementData, DrawSpecification>&)
		{
			static_assert(false);
		};

		template<typename VertexData, typename ElementData, typename DrawSpecification>
		void draw(const Batch<VertexData, ElementData, DrawSpecification>&, size_t)
		{
			static_assert(false);
		};
	}
}
