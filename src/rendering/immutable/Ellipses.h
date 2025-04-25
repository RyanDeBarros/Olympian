#pragma once

#include "../SpecializedBuffers.h"
#include "math/DataStructures.h"
#include "math/Transforms.h"
#include "util/IDGenerator.h"

#include <unordered_set>

// LATER this design is extremely similar to TextureQuadBatch. Maybe it can be abstracted.
namespace oly
{
	namespace immut
	{
		struct Ellipse;
	
		class EllipseBatch
		{
			friend struct Ellipse;

			rendering::VertexArray vao;
			mutable rendering::QuadLayoutEBO<rendering::Mutability::IMMUTABLE> ebo;
			
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
			rendering::IndexedSSBO<EllipseDimension, GLushort, rendering::Mutability::IMMUTABLE> dimension_ssbo;
			rendering::IndexedSSBO<ColorGradient, GLushort, rendering::Mutability::IMMUTABLE> color_ssbo;
			rendering::IndexedSSBO<glm::mat3, GLushort, rendering::Mutability::IMMUTABLE> transform_ssbo;

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
			typedef GLushort EllipsePos;

			EllipseBatch(Capacity capacity, const glm::vec4& projection_bounds);

			void draw(size_t draw_spec = 0) const;
			void draw(Range<EllipsePos> range) const;

			glm::vec4 projection_bounds;

			std::vector<Range<EllipsePos>> draw_specs;

		private:
			mutable math::IndexBijection<EllipsePos> z_order;
			StrictIDGenerator<EllipsePos> pos_generator;
			typedef StrictIDGenerator<EllipsePos>::ID EID;

		public:
			class EllipseReference
			{
				friend EllipseBatch;
				EllipseBatch* _batch = nullptr;
				EID pos;
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

				EllipsePos index_pos() const { return _batch->z_order.range_of(pos.get()); }
				void set_z_index(EllipsePos z) { _batch->move_ellipse_order(index_pos(), z); }
				void move_z_index(int by) { _batch->move_ellipse_order(index_pos(), index_pos() + by); }

				void send_dimension() const;
				void send_color() const;
				void send_transform() const;
				void send_data() const;
				void send_z_value() { _batch->dirty_z = true; }
			};
			friend class EllipseReference;

			void swap_ellipse_order(EllipsePos pos1, EllipsePos pos2) const;
			void move_ellipse_order(EllipsePos from, EllipsePos to) const;

		private:
			void flush() const;
			mutable bool dirty_z = false;
			mutable std::vector<EllipseReference*> ellipse_refs;
			std::unordered_set<Ellipse*> ellipses;
			void flush_z_values() const;
		};

		struct Ellipse
		{
			EllipseBatch::EllipseReference ellipse;
			Transformer2D transformer;

			Ellipse(EllipseBatch* ellipse_batch);
			Ellipse(const Ellipse&) = delete;
			Ellipse(Ellipse&&) noexcept;
			~Ellipse();
			Ellipse& operator=(Ellipse&&) noexcept;

			const EllipseBatch& batch() const { return ellipse.batch(); }
			EllipseBatch& batch() { return ellipse.batch(); }
			const Transform2D& local() const { return transformer.local; }
			Transform2D& local() { return transformer.local; }
			void post_set(); // call after modifying local
			void pre_get() const; // call before reading global

			void draw_unit() const;

		private:
			friend class EllipseBatch;
			void flush();
		};
	}
}
