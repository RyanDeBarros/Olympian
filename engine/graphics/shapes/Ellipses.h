#pragma once

#include <unordered_set>

#include "core/base/Transforms.h"
#include "core/base/Constants.h"
#include "core/containers/IDGenerator.h"
#include "core/types/SmartReference.h"

#include "graphics/BatchBarrier.h"
#include "graphics/backend/basic/VertexArrays.h"
#include "graphics/backend/specialized/ElementBuffers.h"

// TODO v4 can create a Painter system that allows for drawing on a framebuffer -> texture on a separate thread.
// This would happen once per ellipse, and then just use it as a texture in a Sprite, rather than drawing in a separate batch.
// Can even re-use say, a giant white ellipse texture that can be scaled down and colored via modulation in sprite.
// Only keep ellipse batcher for use in collision debug drawing, but perhaps combine ellipse and polygon shaders at that point.
// Also, remove ellipse/polygon batch from context - they can be used manually.

namespace oly::rendering
{
	struct Ellipse;
	class EllipseBatch;

	constexpr EllipseBatch* CONTEXT_ELLIPSE_BATCH = nullptr;

	class EllipseBatch
	{
		friend struct Ellipse;

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
		struct Capacity
		{
			Capacity(Index ellipses = 1) : ellipses(ellipses) { OLY_ASSERT(4 * ellipses <= nmax<unsigned int>()); }

		private:
			friend class EllipseBatch;
			Index ellipses;
		};

	public:
		EllipseBatch(Capacity capacity = {});
		EllipseBatch(const EllipseBatch&) = delete;
		EllipseBatch(EllipseBatch&&) = delete;

		void render() const;

		glm::mat3 projection = 1.0f;

	private:
		StrictIDGenerator<Index> pos_generator;
		typedef StrictIDGenerator<Index>::ID EllipseID;
		EllipseID generate_id();

	public:
		class EllipseReference
		{
			friend class EllipseBatch;
			friend struct Ellipse;
			EllipseBatch& batch;
			const bool in_context;
			EllipseID pos;

		public:
			EllipseReference(EllipseBatch* batch = CONTEXT_ELLIPSE_BATCH);
			EllipseReference(const EllipseReference&);
			EllipseReference(EllipseReference&&) noexcept;
			EllipseReference& operator=(const EllipseReference&);
			EllipseReference& operator=(EllipseReference&&) noexcept;

			const EllipseDimension& get_dimension() const;
			EllipseDimension& set_dimension();
			const ColorGradient& get_color() const;
			ColorGradient& set_color();
			const glm::mat3& get_transform() const;
			glm::mat3& set_transform();

			void draw(BatchBarrier barrier = batch::BARRIER) const;
		};
		friend class EllipseReference;
	};

	struct Ellipse
	{
		EllipseBatch::EllipseReference ellipse;
		Transformer2D transformer;

		Ellipse(EllipseBatch* batch = CONTEXT_ELLIPSE_BATCH) : ellipse(batch) {}
		Ellipse(float r, glm::vec4 color = glm::vec4(1.0f));
		Ellipse(float rx, float ry, glm::vec4 color = glm::vec4(1.0f));
		Ellipse(const Ellipse&) = default;
		Ellipse(Ellipse&&) noexcept = default;
		Ellipse& operator=(const Ellipse&) = default;
		Ellipse& operator=(Ellipse&&) noexcept = default;

		const Transform2D& get_local() const { return transformer.get_local(); }
		Transform2D& set_local() { return transformer.set_local(); }

		void draw(BatchBarrier barrier = batch::BARRIER) const;

		void set_color(glm::vec4 color);
	};

	typedef SmartReference<Ellipse> EllipseRef;
}
