#pragma once

#include <unordered_set>

#include "core/base/Transforms.h"
#include "core/base/Constants.h"
#include "core/containers/IDGenerator.h"
#include "core/types/SmartReference.h"

#include "graphics/backend/basic/VertexArrays.h"
#include "graphics/backend/specialized/ElementBuffers.h"

namespace oly::rendering
{
	class EllipseReference;

	class EllipseBatch
	{
		friend class EllipseReference;

	public:
		using Index = GLuint;

	private:
		graphics::VertexArray vao;
		graphics::PersistentEBO<6> ebo;
			
	public:
		struct EllipseDimension
		{
			float rx = 0.0f, ry = 0.0f, border = 0.0f;
			float fill_exp = 0.0f, border_exp = 0.0f;
		};

		struct ColorGradient {
			glm::vec4 fill_inner = glm::vec4(1.0f);
			glm::vec4 fill_outer = glm::vec4(1.0f);
			glm::vec4 border_inner = glm::vec4(1.0f);
			glm::vec4 border_outer = glm::vec4(1.0f);
		};

	private:
		enum
		{
			DIMENSION,
			COLOR,
			TRANSFORM
		};
		graphics::LazyPersistentGPUBufferBlock<EllipseDimension, ColorGradient, glm::mat3> ssbo_block;

		GLuint projection_location;

	public:
		EllipseBatch();
		EllipseBatch(const EllipseBatch&) = delete;
		EllipseBatch(EllipseBatch&&) = delete;

		void render() const;

		glm::mat3 projection = 1.0f;

	private:
		StrictIDGenerator<Index> pos_generator;
		typedef StrictIDGenerator<Index>::ID EllipseID;
		EllipseID generate_id();
	};

	class EllipseReference
	{
		friend class EllipseBatch;
		EllipseBatch* batch = nullptr;
		EllipseBatch::EllipseID pos;

	public:
		EllipseReference() = default;
		EllipseReference(EllipseBatch& batch);
		EllipseReference(const EllipseReference&);
		EllipseReference(EllipseReference&&) noexcept;
		EllipseReference& operator=(const EllipseReference&);
		EllipseReference& operator=(EllipseReference&&) noexcept;

		EllipseBatch* get_batch() const { return batch; }
		void set_batch(EllipseBatch* batch);

		EllipseBatch::EllipseDimension get_dimension() const;
		EllipseBatch::EllipseDimension& set_dimension();
		const EllipseBatch::ColorGradient& get_color() const;
		EllipseBatch::ColorGradient& set_color();
		const glm::mat3& get_transform() const;
		glm::mat3& set_transform();

		void draw() const;

	private:
		struct Attributes
		{
			EllipseBatch::EllipseDimension dimension = {};
			EllipseBatch::ColorGradient color = {};
			glm::mat3 transform = 1.0f;
		};

		Attributes get_attributes() const
		{
			return {
				.dimension = get_dimension(),
				.color = get_color(),
				.transform = get_transform()
			};
		}

		void set_attributes(const Attributes& attr = {})
		{
			set_dimension() = attr.dimension;
			set_color() = attr.color;
			set_transform() = attr.transform;
		}
	};

	struct Ellipse
	{
		EllipseReference ellipse;
		Transformer2D transformer;

		Ellipse() = default;
		Ellipse(EllipseBatch& batch) : ellipse(batch) {}
		Ellipse(EllipseBatch& batch, float r, glm::vec4 color = glm::vec4(1.0f));
		Ellipse(EllipseBatch& batch, float rx, float ry, glm::vec4 color = glm::vec4(1.0f));
		Ellipse(const Ellipse&) = default;
		Ellipse(Ellipse&&) noexcept = default;
		Ellipse& operator=(const Ellipse&) = default;
		Ellipse& operator=(Ellipse&&) noexcept = default;

		const Transform2D& get_local() const { return transformer.get_local(); }
		Transform2D& set_local() { return transformer.set_local(); }

		void draw() const;

		void set_color(glm::vec4 color);
	};

	typedef SmartReference<Ellipse> EllipseRef;
}
