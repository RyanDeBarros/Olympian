#pragma once

#include "../SpecializedBuffers.h"
#include "math/DataStructures.h"
#include "math/Transforms.h"
#include "util/IDGenerator.h"

#include <unordered_set>

namespace oly
{
	namespace rendering
	{
		struct Ellipse;
	
		class EllipseBatch
		{
			friend struct Ellipse;

		public:
			using Index = GLuint;

		private:
			VertexArray vao;
			mutable LazyPersistentGPUBuffer<std::array<Index, 6>> ebo; // TODO abstract persistent EBO
			
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
			struct SSBO
			{
				LazyPersistentGPUBuffer<EllipseDimension> dimension;
				LazyPersistentGPUBuffer<ColorGradient> color;
				LazyPersistentGPUBuffer<glm::mat3> transform;

				SSBO(Index ellipses) : dimension(ellipses), color(ellipses), transform(ellipses) {}
			} ssbo;

			GLuint projection_location;

		public:
			struct Capacity
			{
				Index ellipses;

				Capacity(Index ellipses)
					: ellipses(ellipses)
				{
					OLY_ASSERT(4 * ellipses <= UINT_MAX);
				}
			};

		private:
			Capacity capacity;

		public:
			EllipseBatch(Capacity capacity, const glm::vec4& projection_bounds);

			void render() const;

			glm::vec4 projection_bounds;

		private:
			void grow_ssbos();
			void grow_ebo() const;

			StrictIDGenerator<Index> pos_generator;
			typedef StrictIDGenerator<Index>::ID EllipseID;

		public:
			class EllipseReference
			{
				friend EllipseBatch;
				EllipseBatch* _batch = nullptr;
				EllipseID pos;

			public:
				EllipseReference(EllipseBatch* batch);
				EllipseReference(const EllipseReference&) = default;
				EllipseReference(EllipseReference&&) noexcept = default;
				EllipseReference& operator=(const EllipseReference&) = default;
				EllipseReference& operator=(EllipseReference&&) noexcept = default;

				EllipseBatch& batch() const { return *_batch; }
				EllipseDimension& dimension() const { return _batch->ssbo.dimension.buf[pos.get()]; }
				ColorGradient& color() const { return _batch->ssbo.color.buf[pos.get()]; }
				glm::mat3& transform() const { return _batch->ssbo.transform.buf[pos.get()]; }

				void flag_dimension() const;
				void flag_color() const;
				void flag_transform() const;
			};
			friend class EllipseReference;

		private:
			mutable GLuint num_ellipses_to_draw = 0;
		};

		struct Ellipse
		{
			EllipseBatch::EllipseReference ellipse;
			Transformer2D transformer;

			Ellipse(EllipseBatch* ellipse_batch) : ellipse(ellipse_batch) {}

			const EllipseBatch& batch() const { return ellipse.batch(); }
			EllipseBatch& batch() { return ellipse.batch(); }
			const Transform2D& local() const { return transformer.local; }
			Transform2D& local() { return transformer.local; }
			void post_set() const { transformer.post_set(); } // call after modifying local
			void pre_get() const { transformer.pre_get(); } // call before reading global

			void draw() const;
		};
	}
}
