#pragma once

#include "core/base/Transforms.h"
#include "core/base/Constants.h"
#include "core/cmath/ColoredGeometry.h"
#include "core/containers/FreeSpaceTracker.h"
#include "core/containers/IDGenerator.h"
#include "core/types/SmartReference.h"

#include "graphics/backend/specialized/ElementBuffers.h"
#include "graphics/backend/specialized/VertexBuffers.h"

namespace oly::rendering
{
	namespace internal
	{
		class PolygonReference;
	}

	class PolygonBatch
	{
		friend class internal::PolygonReference;

		GLuint projection_location;

		graphics::VertexArray vao;
		graphics::PersistentEBO<1> ebo;

		enum
		{
			POSITION,
			COLOR,
			INDEX
		};
		graphics::PersistentVertexBufferBlock<glm::vec2, glm::vec4, GLuint> vbo_block;

		graphics::LazyPersistentGPUBuffer<glm::mat3> transform_ssbo;

	public:
		typedef GLuint Index;
		// ID refers to the index of a polygon in transform SSBO. It also indexes a set of ranges in the VBO block.
		typedef StrictIDGenerator<Index>::ID PolygonID;

		PolygonBatch();
		PolygonBatch(const PolygonBatch&) = delete;
		PolygonBatch(PolygonBatch&&) = delete;

		void render() const;
			
		glm::mat3 projection = 1.0f;

	private:
		void set_primitive_points(Range<Index> vbo_range, const glm::vec2* points, Index count);
		void set_primitive_colors(Range<Index> vbo_range, const glm::vec4* colors, Index count);
		void set_polygon_points(const PolygonID& id, const cmath::Polygon2D& polygon);
		void set_polygon_points(const PolygonID& id, const std::vector<cmath::Polygon2D>& polygons);
		void set_polygon_points(const PolygonID& id, const cmath::Polygon2DComposite& composite);
		void set_polygon_colors(const PolygonID& id, const cmath::Polygon2D& polygon);
		void set_polygon_colors(const PolygonID& id, const std::vector<cmath::Polygon2D>& polygons);
		void set_polygon_colors(const PolygonID& id, const cmath::Polygon2DComposite& composite);
		void set_polygon(const PolygonID& id, const cmath::Polygon2D& polygon);
		void set_polygon(const PolygonID& id, const std::vector<cmath::Polygon2D>& polygons);
		void set_polygon(const PolygonID& id, const cmath::Polygon2DComposite& composite);
		void set_polygon_transform(const PolygonID& id, const glm::mat3& transform);
		const glm::mat3& get_polygon_transform(const PolygonID& id);

		PolygonID generate_id(Index vertices);
		void terminate_id(const PolygonID& id);
		bool resize_range(PolygonID& id, Index vertices);
		Range<Index> get_vertex_range(const PolygonID& id) const;
		bool is_valid_id(const PolygonID& id) const;

		StrictFreeSpaceTracker<Index> vertex_free_space;
		std::unordered_map<Index, Range<Index>> polygon_indexer;
		StrictIDGenerator<Index> id_generator;
	};

	namespace internal
	{
		class PolygonReference
		{
			friend class PolygonBatch;
			PolygonBatch* batch = nullptr;
			mutable PolygonBatch::PolygonID id;
			
		public:
			PolygonReference(PolygonBatch* batch = nullptr);
			PolygonReference(const PolygonReference&);
			PolygonReference(PolygonReference&&) noexcept;
			~PolygonReference();
			PolygonReference& operator=(const PolygonReference&);
			PolygonReference& operator=(PolygonReference&&) noexcept;

			PolygonBatch* get_batch() const { return batch; }
			void set_batch(PolygonBatch* batch);

			bool resize_range(PolygonBatch::Index vertices) const;
			Range<PolygonBatch::Index> get_vertex_range() const;

			void set_polygon_points(const cmath::Polygon2D& polygon) const;
			void set_polygon_points(const std::vector<cmath::Polygon2D>& polygons) const;
			void set_polygon_points(const cmath::Polygon2DComposite& composite) const;
			void set_polygon_colors(const cmath::Polygon2D& polygon) const;
			void set_polygon_colors(const std::vector<cmath::Polygon2D>& polygons) const;
			void set_polygon_colors(const cmath::Polygon2DComposite& composite) const;
			void set_polygon(const cmath::Polygon2D& polygon) const;
			void set_polygon(const std::vector<cmath::Polygon2D>& polygons) const;
			void set_polygon(const cmath::Polygon2DComposite& composite) const;
			void set_polygon_transform(const glm::mat3& transform) const;

			GLuint& draw_index() const;
		};
	}

	// ASSET
	class StaticPolygon
	{
		internal::PolygonReference ref;
		mutable math::Triangulation triangulation;
		cmath::Polygon2D polygon;

		struct
		{
			bool points = true;
			bool colors = true;
		} mutable dirty;

	public:
		StaticPolygon(PolygonBatch* batch = nullptr);
		StaticPolygon(const StaticPolygon&);
		StaticPolygon(StaticPolygon&&) noexcept = default;
		StaticPolygon& operator=(const StaticPolygon&);
		StaticPolygon& operator=(StaticPolygon&&) noexcept = default;

		PolygonBatch* get_batch() const { return ref.get_batch(); }
		void set_batch(PolygonBatch* batch) { ref.set_batch(batch); }

		const cmath::Polygon2D& get_polygon() const { return polygon; }
		const std::vector<glm::vec2>& get_points() const { return polygon.points; }
		const std::vector<glm::vec4>& get_colors() const { return polygon.colors; }

		cmath::Polygon2D& set_polygon() { dirty.points = true; dirty.colors = true; return polygon; }
		std::vector<glm::vec2>& set_points() { dirty.points = true; return polygon.points; }
		std::vector<glm::vec4>& set_colors() { dirty.colors = true; return polygon.colors; }

		void draw() const;

	private:
		void submit_dirty() const;
	};

	class Polygonal
	{
		internal::PolygonReference ref;

		struct
		{
			bool points = true;
			bool colors = true;
		} mutable dirty;

	public:
		Transformer2D transformer;

		Polygonal(PolygonBatch* batch = nullptr);
		Polygonal(const Polygonal&);
		Polygonal(Polygonal&&) noexcept = default;
		virtual ~Polygonal() = default;
		Polygonal& operator=(const Polygonal&);
		Polygonal& operator=(Polygonal&&) noexcept = default;

		PolygonBatch* get_batch() const { return ref.get_batch(); }
		void set_batch(PolygonBatch* batch) { ref.set_batch(batch); }

		const Transform2D& get_local() const { return transformer.get_local(); }
		Transform2D& set_local() { return transformer.set_local(); }

		virtual GLuint num_vertices() const = 0;
		void draw() const;

	private:
		void submit_dirty() const;

	protected:
		void flag_dirty_points() const { dirty.points = true; }
		void flag_dirty_colors() const { dirty.colors = true; }
		void flag_dirty() const { dirty.points = true;  dirty.colors = true; }

		void set_polygon_points(const cmath::Polygon2D& polygon) const { ref.set_polygon_points(polygon); }
		void set_polygon_points(const std::vector<cmath::Polygon2D>& polygons) const { ref.set_polygon_points(polygons); }
		void set_polygon_points(const cmath::Polygon2DComposite& composite) const { ref.set_polygon_points(composite); }
		void set_polygon_colors(const cmath::Polygon2D& polygon) const { ref.set_polygon_colors(polygon); }
		void set_polygon_colors(const std::vector<cmath::Polygon2D>& polygons) const { ref.set_polygon_colors(polygons); }
		void set_polygon_colors(const cmath::Polygon2DComposite& composite) const { ref.set_polygon_colors(composite); }
		void set_polygon(const cmath::Polygon2D& polygon) const { ref.set_polygon(polygon); }
		void set_polygon(const std::vector<cmath::Polygon2D>& polygons) const { ref.set_polygon(polygons); }
		void set_polygon(const cmath::Polygon2DComposite& composite) const { ref.set_polygon(composite); }

		GLuint& draw_index() const;
		
		virtual void impl_set_polygon() const = 0;
		virtual void impl_set_polygon_points() const = 0;
		virtual void impl_set_polygon_colors() const = 0;
		virtual void triangulate() const = 0;
		virtual void draw_triangulation(GLuint initial_vertex) const = 0;
	};

	class Polygon : public Polygonal
	{
		cmath::Polygon2D polygon;

	public:
		Polygon(PolygonBatch* batch = nullptr);
		Polygon(const Polygon&);
		Polygon(Polygon&&) noexcept;
		Polygon& operator=(const Polygon&);
		Polygon& operator=(Polygon&&) noexcept;

		const cmath::Polygon2D& get_polygon() const { return polygon; }
		const std::vector<glm::vec2>& get_points() const { return polygon.points; }
		const std::vector<glm::vec4>& get_colors() const { return polygon.colors; }

		cmath::Polygon2D& set_polygon() { flag_dirty(); return polygon; }
		std::vector<glm::vec2>& set_points() { flag_dirty_points(); return polygon.points; }
		std::vector<glm::vec4>& set_colors() { flag_dirty_colors(); return polygon.colors; }

		virtual GLuint num_vertices() const override;

	private:
		mutable math::Triangulation cache;

	protected:
		virtual void impl_set_polygon() const override;
		virtual void impl_set_polygon_points() const override;
		virtual void impl_set_polygon_colors() const override;
		virtual void triangulate() const override;
		virtual void draw_triangulation(GLuint initial_vertex) const override;
	};

	class PolyComposite : public Polygonal
	{
		cmath::Polygon2DComposite composite;

	public:
		PolyComposite(PolygonBatch* batch = nullptr);
		PolyComposite(const PolyComposite&);
		PolyComposite(PolyComposite&&) noexcept;
		PolyComposite& operator=(const PolyComposite&);
		PolyComposite& operator=(PolyComposite&&) noexcept;

		const cmath::Polygon2DComposite& get_composite() const { return composite; }
		const cmath::TriangulatedPolygon2D& get_triangulated_polygon(size_t i) const { return composite[i]; }
		const math::Triangulation& get_triangulation(size_t i) const { return composite[i].triangulation; }
		const cmath::Polygon2D& get_polygon(size_t i) const { return composite[i].polygon; }
		const std::vector<glm::vec2>& get_points(size_t i) const { return composite[i].polygon.points; }
		const std::vector<glm::vec4>& get_colors(size_t i) const { return composite[i].polygon.colors; }

		cmath::Polygon2DComposite& set_composite() { flag_dirty(); return composite; }
		cmath::TriangulatedPolygon2D& set_triangulated_polygon(size_t i) { flag_dirty(); return composite[i]; }
		math::Triangulation& set_triangulation(size_t i) { flag_dirty_points(); return composite[i].triangulation; }
		cmath::Polygon2D& set_polygon(size_t i) { flag_dirty(); return composite[i].polygon; }
		std::vector<glm::vec2>& set_points(size_t i) { flag_dirty_points(); return composite[i].polygon.points; }
		std::vector<glm::vec4>& set_colors(size_t i) { flag_dirty_colors(); return composite[i].polygon.colors; }

		virtual GLuint num_vertices() const override;

	protected:
		virtual void impl_set_polygon() const override;
		virtual void impl_set_polygon_points() const override;
		virtual void impl_set_polygon_colors() const override;
		virtual void triangulate() const override;
		virtual void draw_triangulation(GLuint initial_vertex) const override;
	};

	class NGon : public Polygonal
	{
		bool bordered = false;
		cmath::NGonBase base;

	public:
		NGon(PolygonBatch* batch = nullptr);
		NGon(const NGon&);
		NGon(NGon&&) noexcept;
		NGon& operator=(const NGon&);
		NGon& operator=(NGon&&) noexcept;

		bool is_bordered() const { return bordered; }
		const cmath::NGonBase& get_base() const { return base; }
		const std::vector<glm::vec2>& get_points() const { return base.points; }
		const std::vector<glm::vec4>& get_fill_colors() const { return base.fill_colors; }
		const std::vector<glm::vec4>& get_border_colors() const { return base.border_colors; }
		float get_border_width() const { return base.border_width; }
		cmath::BorderPivot get_border_pivot() const { return base.border_pivot; }

		void set_bordered(bool b) { flag_dirty(); bordered = b; }
		cmath::NGonBase& set_base() { flag_dirty(); return base; }
		std::vector<glm::vec2>& set_points() { flag_dirty_points(); return base.points; }
		std::vector<glm::vec4>& set_fill_colors() { flag_dirty_colors(); return base.fill_colors; }
		std::vector<glm::vec4>& set_border_colors() { flag_dirty_colors(); return base.border_colors; }
		void set_border_width(float bw) { flag_dirty_points(); base.border_width = bw; }
		void set_border_pivot(cmath::BorderPivot pivot) { flag_dirty_points(); base.border_pivot = pivot; }

		virtual GLuint num_vertices() const override;

	private:
		mutable cmath::Polygon2DComposite cache;

	protected:
		virtual void impl_set_polygon() const override;
		virtual void impl_set_polygon_points() const override;
		virtual void impl_set_polygon_colors() const override;
		virtual void triangulate() const override;
		virtual void draw_triangulation(GLuint initial_vertex) const override;
	};
}

namespace oly
{
	namespace rendering { typedef SmartReference<Polygonal> PolygonalRef; }

	OLY_SMART_POOL_BASE(rendering::Polygon, rendering::Polygonal);
	namespace rendering { typedef SmartReference<Polygon> PolygonRef; }

	OLY_SMART_POOL_BASE(rendering::PolyComposite, rendering::Polygonal);
	namespace rendering { typedef SmartReference<PolyComposite> PolyCompositeRef; }

	OLY_SMART_POOL_BASE(rendering::NGon, rendering::Polygonal);
	namespace rendering { typedef SmartReference<NGon> NGonRef; }
}
