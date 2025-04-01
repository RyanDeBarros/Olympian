#pragma once

#include "SpecializedBuffers.h"
#include "math/Transforms.h"
#include "math/DataStructures.h"
#include "math/Geometry.h"
#include "util/FixedVector.h"

namespace oly
{
	namespace renderable
	{
		class Polygonal;
	}

	namespace batch
	{
		class PolygonBatch
		{
			friend renderable::Polygonal;
			GLuint shader;
			rendering::VertexArray vao;
			rendering::FixedLayoutEBO<GLushort, 1> ebo;

			GLuint projection_location;
			GLuint degree_location;

			enum PolygonAttribute
			{
				POSITION,
				COLOR
			};
			rendering::LazyVertexBufferBlock2x1<glm::vec2, glm::vec4, GLushort> polygon_vbo;

			rendering::IndexedSSBO<glm::mat3, GLushort> transform_ssbo;

		public:
			typedef GLushort PrimitivePos;
			typedef GLushort PolygonPos;

			struct Capacity
			{
				GLushort vertices = 0;
				GLushort indices = 0;
				GLushort polygons = 0;
				GLushort degree = 0;
				GLushort polygon_index_count = 0;

				Capacity(GLushort polygons, GLushort degree = 6)
					: polygons(polygons), degree(degree)
				{
					assert(degree >= 3);
					assert(degree * polygons <= USHRT_MAX);

					// max(F) = V - 2 + 2H
					// max(H) = [V / 3] - 1
					// --> max(F) = V + 2 * [V / 3] - 4
					// --> return 3 * max(F)
					polygon_index_count = 3 * degree + 6 * (degree / 3) - 12;
					vertices = polygons * degree;
					indices = polygons * polygon_index_count;
				}
			};

		private:
			const Capacity capacity;

		public:
			GLushort max_degree() const { return capacity.degree; }

			PolygonBatch(Capacity capacity, const glm::vec4& projection_bounds);

			void draw() const;

			void get_primitive_draw_spec(PrimitivePos& first, PrimitivePos& count) const;
			void set_primitive_draw_spec(PrimitivePos first, PrimitivePos count);

			void set_projection(const glm::vec4& projection_bounds) const;

			void set_primitive_points(PrimitivePos pos, const glm::vec2* points, GLushort count);
			void set_primitive_colors(PrimitivePos pos, const glm::vec4* colors, GLushort count);
			void set_primitive_transform(PrimitivePos pos, const glm::mat3& transform);
			void set_primitive_triangulation(PrimitivePos pos, const math::Triangulation& triangulation);
			void set_polygon_primitive(PrimitivePos pos, const math::TriangulatedPolygon2D& polygon, const glm::mat3& transform);
			void disable_polygon_primitive(PrimitivePos pos);

			void set_range_transform(Range<GLushort> range, const glm::mat3& transform);
			void disable_polygon_range(Range<GLushort> range);
			void disable_polygon(PolygonPos pos);

			Range<GLushort> set_polygon(PolygonPos pos, const math::TriangulatedPolygon2D& polygon, const glm::mat3& transform, GLushort min_range = 0, GLushort max_range = 0);
			Range<GLushort> set_polygon(PolygonPos pos, math::TriangulatedPolygon2D&& polygon, const glm::mat3& transform, GLushort min_range = 0, GLushort max_range = 0);
			Range<GLushort> set_polygon(PolygonPos pos, const math::Polygon2DComposite& composite, const glm::mat3& transform, GLushort min_range = 0, GLushort max_range = 0);
			Range<GLushort> set_polygon(PolygonPos pos, math::Polygon2DComposite&& composite, const glm::mat3& transform, GLushort min_range = 0, GLushort max_range = 0);
			Range<GLushort> set_ngon(PolygonPos pos, const math::NGonBase& ngon, const glm::mat3& transform, GLushort min_range = 0, GLushort max_range = 0);
			Range<GLushort> set_bordered_ngon(PolygonPos pos, const math::NGonBase& ngon, const glm::mat3& transform, GLushort min_range = 0, GLushort max_range = 0);

			// TODO some kind of remove_polygon/remove_composite that removes range and updates PolygonIndexer somehow. Change PolygonIndexer to FreeSpaceTracker that only tracks slots that are open.

			math::Polygon2DComposite create_bordered_ngon(glm::vec4 fill_color, glm::vec4 border_color, float border, math::BorderPivot border_pivot, const std::vector<glm::vec2>& points) const;
			math::Polygon2DComposite create_bordered_ngon(glm::vec4 fill_color, glm::vec4 border_color, float border, math::BorderPivot border_pivot, std::vector<glm::vec2>&& points) const;
			math::Polygon2DComposite create_bordered_ngon(const std::vector<glm::vec4>& fill_colors, const std::vector<glm::vec4>& border_colors,
				float border, math::BorderPivot border_pivot, const std::vector<glm::vec2>& points) const;
			math::Polygon2DComposite create_bordered_ngon(std::vector<glm::vec4>&& fill_colors, std::vector<glm::vec4>&& border_colors,
				float border, math::BorderPivot border_pivot, std::vector<glm::vec2>&& points) const;

			math::Polygon2DComposite create_ngon(const math::NGonBase& ngon) const;
			math::Polygon2DComposite create_bordered_ngon(const math::NGonBase& ngon) const;

		private:
			struct PolygonIndexer
			{
				struct Composite
				{
					PrimitivePos start;
					GLushort range;
				};

				GLushort total_range = 0;
				std::vector<Composite> composites;

				PrimitivePos next_pos() const { return total_range; }
				void register_composite(PrimitivePos start, GLushort range)
				{
					composites.emplace_back(start, range);
					total_range += range;
				}
				PrimitivePos get_pos(PolygonPos pos) const { return composites[pos].start; }
				GLushort get_range(PolygonPos pos) const { return composites[pos].range; }
				bool exists(PolygonPos pos) const { return pos < composites.size(); }
				GLushort size() const { return (GLushort)composites.size(); }
			};
			PolygonIndexer polygon_indexer;

			std::unordered_set<renderable::Polygonal*> polygonal_renderables;

		public:
			void flush() const;
		};
	}

	namespace renderable
	{
		class Polygonal
		{
		protected:
			friend batch::PolygonBatch;
			batch::PolygonBatch* _batch = nullptr;
			std::unique_ptr<Transformer2D> _transformer = nullptr;
			Range<GLushort> range = {};

		public:
			Polygonal(batch::PolygonBatch* batch);
			Polygonal(batch::PolygonBatch* batch, const Transform2D& local);
			Polygonal(batch::PolygonBatch* batch, std::unique_ptr<Transformer2D>&& transformer);
			Polygonal(const Polygonal&) = delete;
			Polygonal(Polygonal&&) noexcept;
			virtual ~Polygonal();
			Polygonal& operator=(Polygonal&&) noexcept;

			const batch::PolygonBatch* batch() const { return _batch; }
			batch::PolygonBatch* batch() { return _batch; }
			Range<GLushort> get_range() const { return range; }
			const Transformer2D& transformer() const { return *_transformer; }
			Transformer2D& transformer() { return *_transformer; }
			const Transform2D& local() const { return _transformer->local; }
			Transform2D& local() { return _transformer->local; }
			void post_set() const; // call after modifying local
			void pre_get() const; // call before reading global

			virtual void send_polygon() const = 0;
		
		protected:
			void init(batch::PolygonBatch::PolygonPos pos, const math::Polygon2DComposite& composite, GLushort min_range, GLushort max_range);
			void init(batch::PolygonBatch::PolygonPos pos, math::Polygon2DComposite&& composite, GLushort min_range, GLushort max_range);
			void send_polygon(const math::Polygon2DComposite& composite) const;

		private:
			void flush() const;
		};

		struct Polygon : public Polygonal
		{
			math::Polygon2D polygon;

			using Polygonal::Polygonal;
			Polygon(Polygon&&) noexcept = default;
			Polygon& operator=(Polygon&&) noexcept = default;

			void init(batch::PolygonBatch::PolygonPos pos, GLushort min_range = 0, GLushort max_range = 0);

			virtual void send_polygon() const override;
		};

		struct Composite : public Polygonal
		{
			math::Polygon2DComposite composite;

			using Polygonal::Polygonal;
			Composite(Composite&&) noexcept = default;
			Composite& operator=(Composite&&) noexcept = default;

			void init(batch::PolygonBatch::PolygonPos pos, GLushort min_range = 0, GLushort max_range = 0);

			virtual void send_polygon() const override;
		};

		struct NGon : public Polygonal
		{
		private:
			bool bordered = false;
		
		public:
			math::NGonBase base;

			using Polygonal::Polygonal;
			NGon(NGon&&) noexcept = default;
			NGon& operator=(NGon&&) noexcept = default;

			void init(batch::PolygonBatch::PolygonPos pos, bool gen_border, GLushort min_range = 0, GLushort max_range = 0);

			virtual void send_polygon() const override;
		};
	}

	namespace batch
	{
		// TODO
		class EllipseBatch;
	}
}
