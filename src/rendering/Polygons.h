#pragma once

#include <map>

#include "SpecializedBuffers.h"
#include "math/Transforms.h"
#include "math/Geometry.h"
#include "util/FixedVector.h"
#include "util/FreeSpaceTracker.h"
#include "util/IDGenerator.h"

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
			typedef GLushort RangeID;

			struct Capacity
			{
				GLushort vertices = 0;
				GLushort indices = 0;
				GLushort primitives = 0;
				GLushort degree = 0;
				GLushort polygon_index_count = 0;

				Capacity(GLushort primitives, GLushort degree = 6)
					: primitives(primitives), degree(degree)
				{
					assert(degree >= 3);
					assert(degree * primitives <= USHRT_MAX);

					// max(F) = V - 2 + 2H
					// max(H) = [V / 3] - 1
					// --> max(F) = V + 2 * [V / 3] - 4
					// --> return 3 * max(F)
					polygon_index_count = 3 * degree + 6 * (degree / 3) - 12;
					vertices = primitives * degree;
					indices = primitives * polygon_index_count;
				}
			};

		private:
			const Capacity capacity;

		public:
			const Capacity& get_capacity() const { return capacity; }

			PolygonBatch(Capacity capacity, const glm::vec4& projection_bounds);

			void draw(size_t draw_spec = 0);

		private:
			void get_primitive_draw_spec(PrimitivePos& first, PrimitivePos& count) const;
			void set_primitive_draw_spec(PrimitivePos first, PrimitivePos count);

		public:
			std::vector<Range<PrimitivePos>> draw_specs;

			void set_projection(const glm::vec4& projection_bounds) const;

		private:
			void set_primitive_points(PrimitivePos pos, const glm::vec2* points, GLushort count);
			void set_primitive_colors(PrimitivePos pos, const glm::vec4* colors, GLushort count);
			void set_primitive_transform(PrimitivePos pos, const glm::mat3& transform);
			void set_primitive_triangulation(PrimitivePos vertex_pos, PrimitivePos index_pos, const math::Triangulation& triangulation);
			void set_polygon_primitive(PrimitivePos vertex_pos, PrimitivePos index_pos, const math::TriangulatedPolygon2D& polygon, const glm::mat3& transform);
			void disable_polygon_primitive(PrimitivePos index_pos);

		public:
			void set_polygon_transform(RangeID id, const glm::mat3& transform);
			void disable_polygon(RangeID id);

		private:
			RangeID generate_id(const math::Polygon2DComposite& composite, GLushort min_range = 0, GLushort max_range = 0);
			RangeID generate_id(math::Polygon2DComposite& composite, GLushort min_range = 0, GLushort max_range = 0);
			void terminate_id(RangeID id);
			void resize_range(RangeID id, const math::Polygon2DComposite& composite, GLushort min_range = 0, GLushort max_range = 0);
			void resize_range(RangeID id, math::Polygon2DComposite& composite, GLushort min_range = 0, GLushort max_range = 0);

		public:
			bool is_valid_id(RangeID id) const;
			bool get_next_draw_id(RangeID id, RangeID& next_id) const;
			bool get_prev_draw_id(RangeID id, RangeID& prev_id) const;
		private:
			Range<PrimitivePos> get_vertex_range(RangeID id) const;
		public:
			Range<PrimitivePos> get_index_range(RangeID id) const;

			void set_polygon(RangeID id, const math::TriangulatedPolygon2D& polygon, const glm::mat3& transform);
			void set_polygon(RangeID id, math::TriangulatedPolygon2D&& polygon, const glm::mat3& transform);
			void set_polygon(RangeID id, const math::Polygon2DComposite& composite, const glm::mat3& transform);
			void set_polygon(RangeID id, math::Polygon2DComposite& composite, const glm::mat3& transform);
			void set_ngon(RangeID id, const math::NGonBase& ngon, const glm::mat3& transform);
			void set_bordered_ngon(RangeID id, const math::NGonBase& ngon, const glm::mat3& transform);

			void swap_poly_order_with_next(RangeID id);
			void swap_poly_order_with_prev(RangeID id);
		
		private:
			void swap_adjacent_poly_orders(Range<PrimitivePos>& left_index_range, Range<PrimitivePos>& right_index_range);

		public:
			void move_poly_order_after(RangeID id, RangeID after);
			void move_poly_order_before(RangeID id, RangeID before);

			math::Polygon2DComposite create_bordered_ngon(glm::vec4 fill_color, glm::vec4 border_color, float border, math::BorderPivot border_pivot, const std::vector<glm::vec2>& points) const;
			math::Polygon2DComposite create_bordered_ngon(glm::vec4 fill_color, glm::vec4 border_color, float border, math::BorderPivot border_pivot, std::vector<glm::vec2>&& points) const;
			math::Polygon2DComposite create_bordered_ngon(const std::vector<glm::vec4>& fill_colors, const std::vector<glm::vec4>& border_colors,
				float border, math::BorderPivot border_pivot, const std::vector<glm::vec2>& points) const;
			math::Polygon2DComposite create_bordered_ngon(std::vector<glm::vec4>&& fill_colors, std::vector<glm::vec4>&& border_colors,
				float border, math::BorderPivot border_pivot, std::vector<glm::vec2>&& points) const;

			math::Polygon2DComposite create_ngon(const math::NGonBase& ngon) const;
			math::Polygon2DComposite create_bordered_ngon(const math::NGonBase& ngon) const;

		private:
			StrictFreeSpaceTracker<PrimitivePos> vertex_free_space, index_free_space;
			struct PolygonIndexer
			{
				Range<PrimitivePos> vertex_range;
				Range<PrimitivePos> index_range;
			};
			std::unordered_map<RangeID, PolygonIndexer> polygon_indexer;
			std::map<PrimitivePos, RangeID> id_order;
			IDGenerator<RangeID> id_generator;

			std::unordered_set<renderable::Polygonal*> polygonal_renderables;

		public:
			void flush() const;
		};
	}

	namespace renderable
	{
		class Polygonal
		{
			friend batch::PolygonBatch;
			batch::PolygonBatch* _batch = nullptr;
			std::unique_ptr<Transformer2D> _transformer = nullptr;
			batch::PolygonBatch::RangeID id = -1;

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
			batch::PolygonBatch::RangeID get_id() const { return id; }
			Range<batch::PolygonBatch::PrimitivePos> index_range() const { return _batch->get_index_range(id); }
			const Transformer2D& transformer() const { return *_transformer; }
			Transformer2D& transformer() { return *_transformer; }
			const Transform2D& local() const { return _transformer->local; }
			Transform2D& local() { return _transformer->local; }
			void post_set() const; // call after modifying local
			void pre_get() const; // call before reading global

			virtual void init(GLushort min_range = 0, GLushort max_range = 0) = 0;
			virtual void resize(GLushort min_range = 0, GLushort max_range = 0) = 0;
			virtual void send_polygon() const = 0;

		protected:
			void init(const math::Polygon2DComposite& composite, GLushort min_range, GLushort max_range);
			void init(math::Polygon2DComposite&& composite, GLushort min_range, GLushort max_range);
			void resize(const math::Polygon2DComposite& composite, GLushort min_range, GLushort max_range);
			void resize(math::Polygon2DComposite&& composite, GLushort min_range, GLushort max_range);
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

			virtual void init(GLushort min_range = 0, GLushort max_range = 0) override;
			virtual void resize(GLushort min_range = 0, GLushort max_range = 0) override;
			virtual void send_polygon() const override;
		};

		struct Composite : public Polygonal
		{
			math::Polygon2DComposite composite;

			using Polygonal::Polygonal;
			Composite(Composite&&) noexcept = default;
			Composite& operator=(Composite&&) noexcept = default;

			void init(GLushort min_range = 0, GLushort max_range = 0) override;
			void resize(GLushort min_range = 0, GLushort max_range = 0) override;
			virtual void send_polygon() const override;
		};

		struct NGon : public Polygonal
		{
			bool bordered = false;
			math::NGonBase base;

			using Polygonal::Polygonal;
			NGon(NGon&&) noexcept = default;
			NGon& operator=(NGon&&) noexcept = default;

			virtual void init(GLushort min_range = 0, GLushort max_range = 0) override;
			virtual void resize(GLushort min_range = 0, GLushort max_range = 0) override;
			virtual void send_polygon() const override;

		private:
			math::Polygon2DComposite composite() const;
		};
	}
}
