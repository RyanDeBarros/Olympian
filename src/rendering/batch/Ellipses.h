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
			PersistentEBO<> ebo;
			
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
			} mutable ssbo;

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
			EllipseBatch(Capacity capacity, const glm::vec4& projection_bounds);

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
				EllipseDimension& dimension() const { return _batch->ssbo.dimension.buf[pos.get()]; }
				ColorGradient& color() const { return _batch->ssbo.color.buf[pos.get()]; }
				glm::mat3& transform() const { return _batch->ssbo.transform.buf[pos.get()]; }

				void flag_dimension() const;
				void flag_color() const;
				void flag_transform() const;
			};
			friend class EllipseReference;
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
