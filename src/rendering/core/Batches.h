#pragma once

#include "rendering/core/Shaders.h"
#include "rendering/core/Buffers.h"
#include "rendering/core/VertexArrays.h"

namespace oly
{
	namespace rendering
	{
		template<typename VertexData, typename ElementData, typename DrawSpecification>
		struct Batch
		{
			VertexData vertex_data;
			ElementData element_data;
			DrawSpecification draw_specification;

			std::shared_ptr<VAODescriptor> vao_descriptor;
			std::shared_ptr<Shader> shader;
		};
	}
}
