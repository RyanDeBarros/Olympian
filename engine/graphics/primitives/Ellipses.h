#pragma once

#include <unordered_set>

#include "core/base/Transforms.h"
#include "core/containers/IDGenerator.h"

#include "graphics/backend/basic/VertexArrays.h"
#include "graphics/backend/specialized/ElementBuffers.h"

namespace oly::rendering
{
	struct Ellipse;

	class EllipseBatch
	{
		friend struct Ellipse;

	public:
		using Index = GLuint;

	private:
		graphics::VertexArray vao;
		graphics::PersistentEBO<> ebo;
			
	public:
		struct EllipseDimension
		{
			float width = 0.0f, height = 0.0f, border = 0.0f;
			float fill_exp = 1.0f, border_exp = 1.0f;
		};
		struct ColorGradient {
			glm::vec4 fill_inner;
			glm::vec4 fill_outer;
			glm::vec4 border_inner;
			glm::vec4 border_outer;
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
		struct Capacity
		{
			Capacity(Index ellipses) : ellipses(ellipses) { OLY_ASSERT(4 * ellipses <= UINT_MAX); }

		private:
			friend class EllipseBatch;
			Index ellipses;
		};

	public:
		EllipseBatch(Capacity capacity);

		void render() const;

		glm::vec4 projection_bounds;

	private:
		StrictIDGenerator<Index> pos_generator;
		typedef StrictIDGenerator<Index>::ID EllipseID;
		EllipseID generate_id();

	public:
		class EllipseReference
		{
			friend class EllipseBatch;
			friend struct Ellipse;
			EllipseBatch* _batch = nullptr;
			EllipseID pos;

		public:
			EllipseReference(EllipseBatch* batch);
			EllipseReference(const EllipseReference&);
			EllipseReference(EllipseReference&&) noexcept = default;
			EllipseReference& operator=(const EllipseReference&);
			EllipseReference& operator=(EllipseReference&&) noexcept = default;

			EllipseBatch& batch() const { return *_batch; }
			const EllipseDimension& get_dimension() const { return _batch->ssbo_block.get<DIMENSION>(pos.get()); }
			EllipseDimension& set_dimension() { return _batch->ssbo_block.set<DIMENSION>(pos.get()); }
			const ColorGradient& get_color() const { return _batch->ssbo_block.get<COLOR>(pos.get()); }
			ColorGradient& set_color() { return _batch->ssbo_block.set<COLOR>(pos.get()); }
			const glm::mat3& get_transform() const { return _batch->ssbo_block.get<TRANSFORM>(pos.get()); }
			glm::mat3& set_transform() { return _batch->ssbo_block.set<TRANSFORM>(pos.get()); }
		};
		friend class EllipseReference;
	};

	struct Ellipse
	{
		EllipseBatch::EllipseReference ellipse;
		Transformer2D transformer;

		Ellipse(EllipseBatch& ellipse_batch) : ellipse(&ellipse_batch) {}
		Ellipse(const Ellipse&) = default;
		Ellipse(Ellipse&&) noexcept = default;
		Ellipse& operator=(const Ellipse&) = default;
		Ellipse& operator=(Ellipse&&) noexcept = default;

		const EllipseBatch& batch() const { return ellipse.batch(); }
		EllipseBatch& batch() { return ellipse.batch(); }
		const Transform2D& get_local() const { return transformer.get_local(); }
		Transform2D& set_local() { return transformer.set_local(); }

		void draw() const;
	};
}
