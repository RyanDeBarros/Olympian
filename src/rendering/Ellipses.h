#pragma once

#include "core/Core.h"
#include "SpecializedBuffers.h"
#include "math/DataStructures.h"
#include "math/Transforms.h"
#include "util/IDGenerator.h"

#include <unordered_set>

// LATER this design is extremely similar to TextureQuadBatch. Maybe it can be abstracted.
namespace oly
{
	namespace renderable
	{
		struct Ellipse;
	}

	namespace batch
	{
		class EllipseBatch
		{
			friend struct renderable::Ellipse;

			GLuint shader;

			rendering::VertexArray vao;
			rendering::QuadLayoutEBO<> ebo;
			
		public:
			struct EllipseDimension
			{
				float width, height, border;
				float fill_exp = 1.0f, border_exp = 1.0f;
			};
			struct ColorGradient {
				glm::vec4 fill_inner;
				glm::vec4 fill_outer;
				glm::vec4 border_inner;
				glm::vec4 border_outer;
			};

		private:
			rendering::IndexedSSBO<EllipseDimension, GLushort> dimension_ssbo;
			rendering::IndexedSSBO<ColorGradient, GLushort> color_ssbo;
			rendering::IndexedSSBO<glm::mat3, GLushort> transform_ssbo;

			GLuint projection_location;

		public:
			struct Capacity
			{
				GLushort ellipses;

				Capacity(GLushort ellipses)
					: ellipses(ellipses)
				{
					OLY_ASSERT(4 * ellipses <= USHRT_MAX);
				}
			};

		private:
			Capacity capacity;

		public:
			EllipseBatch(Capacity capacity, const glm::vec4& projection_bounds);

			void draw(size_t draw_spec = 0);

			void set_projection(const glm::vec4& projection_bounds) const;

			typedef GLushort EllipsePos;
			std::vector<Range<EllipsePos>> draw_specs;

			class EllipseReference
			{
				friend EllipseBatch;
				EllipseBatch* _batch = nullptr;
				EllipsePos pos = -1;
				bool active = true;

			public:
				float z_value = 0.0f;

			private:
				EllipseDimension* _dimension;
				ColorGradient* _color;
				glm::mat3* _transform;

			public:
				EllipseReference(EllipseBatch* batch);
				EllipseReference(const EllipseReference&) = delete;
				EllipseReference(EllipseReference&&) noexcept;
				~EllipseReference();
				EllipseReference& operator=(EllipseReference&&) noexcept;

				const EllipseBatch& batch() const { return *_batch; }
				EllipseBatch& batch() { return *_batch; }
				const EllipseDimension& dimension() const { return *_dimension; }
				EllipseDimension& dimension() { return *_dimension; }
				const ColorGradient& color() const { return *_color; }
				ColorGradient& color() { return *_color; }
				const glm::mat3& transform() const { return *_transform; }
				glm::mat3& transform() { return *_transform; }

			private:
				EllipsePos index_pos() const { return _batch->z_order.range_of(pos); }
				void set_z_index(EllipsePos z) { _batch->move_ellipse_order(index_pos(), z); }
				void move_z_index(int by) { _batch->move_ellipse_order(index_pos(), index_pos() + by); }

			public:
				void send_dimension() const;
				void send_color() const;
				void send_transform() const;
				void send_data() const;
				void send_z_value() { _batch->dirty_z = true; }
			};
			friend EllipseReference;

		private:
			math::IndexBijection<EllipsePos> z_order;
			IDGenerator<EllipsePos> pos_generator;

		public:
			void swap_ellipse_order(EllipsePos pos1, EllipsePos pos2);
			void move_ellipse_order(EllipsePos from, EllipsePos to);

			void flush();

		private:
			bool dirty_z = false;
			std::vector<EllipseReference*> ellipse_refs;
			std::unordered_set<renderable::Ellipse*> ellipses;
			void flush_z_values();
		};
	}

	namespace renderable
	{
		struct Ellipse
		{
			batch::EllipseBatch::EllipseReference ellipse;
			Transformer2D transformer;

			Ellipse(batch::EllipseBatch* ellipse_batch);
			Ellipse(const Ellipse&) = delete;
			Ellipse(Ellipse&&) noexcept;
			~Ellipse();
			Ellipse& operator=(Ellipse&&) noexcept;

			const batch::EllipseBatch& batch() const { return ellipse.batch(); }
			batch::EllipseBatch& batch() { return ellipse.batch(); }
			const Transform2D& local() const { return transformer.local; }
			Transform2D& local() { return transformer.local; }
			void post_set(); // call after modifying local
			void pre_get() const; // call before reading global

		private:
			friend batch::EllipseBatch;
			void flush();
		};
	}
}
