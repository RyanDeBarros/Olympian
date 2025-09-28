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

		typedef GLuint Index;

	public:
		PolygonBatch();
		PolygonBatch(const PolygonBatch&) = delete;
		PolygonBatch(PolygonBatch&&) = delete;

		void render() const;
			
		glm::mat3 projection = 1.0f;

	private:
		void set_primitive_points(Range<Index> vertex_range, const glm::vec2* points, Index count);
		void set_primitive_colors(Range<Index> vertex_range, const glm::vec4* colors, Index count);
		void set_polygon_transform(Index id, const glm::mat3& transform);
		const glm::mat3& get_polygon_transform(Index id);

		Index generate_id(Index vertices);
		void terminate_id(Index id);
		bool resize_range(Index& id, Index vertices);
		Range<Index> get_vertex_range(Index id) const;
		bool is_valid_id(Index id) const;

		StrictFreeSpaceTracker<Index> vertex_free_space;
		std::unordered_map<Index, Range<Index>> polygon_indexer;
		SoftIDGenerator<Index> id_generator;
		static const Index NULL_ID = Index(-1);
		void assert_valid_id(Index id) const;
	};

	namespace internal
	{
		class PolygonReference
		{
			friend class PolygonBatch;
			PolygonBatch* batch = nullptr;
			// ID refers to the index of a polygon in transform SSBO. It also indexes the set of ranges in the VBO block.
			mutable PolygonBatch::Index id = PolygonBatch::NULL_ID;
			
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

			void set_primitive_points(Range<PolygonBatch::Index> vertex_range, const glm::vec2* points, size_t count) const;
			void set_primitive_colors(Range<PolygonBatch::Index> vertex_range, const glm::vec4* colors, size_t count) const;
			void set_primitive_points(const glm::vec2* points, PolygonBatch::Index count) const;
			void set_primitive_colors(const glm::vec4* colors, PolygonBatch::Index count) const;
			void set_polygon_transform(const glm::mat3& transform) const;

			GLuint& draw_index() const;
		};

		class PolygonSubmitter
		{
			PolygonReference ref;
			mutable bool points = true;
			mutable bool colors = true;

		public:
			PolygonSubmitter(PolygonBatch* batch = nullptr);
			PolygonSubmitter(const PolygonSubmitter&);
			PolygonSubmitter(PolygonSubmitter&&) noexcept = default;
			PolygonSubmitter& operator=(const PolygonSubmitter&);
			PolygonSubmitter& operator=(PolygonSubmitter&&) noexcept = default;
			virtual ~PolygonSubmitter() = default;

		protected:
			const internal::PolygonReference& get_ref() const { return ref; }
			void set_batch(PolygonBatch* batch) { ref.set_batch(batch); }

			void flag_points() { points = true; }
			void flag_colors() { colors = true; }
			void flag_all() { points = true; colors = true; }

			void submit_dirty() const;

			virtual void triangulate() const = 0;
			virtual size_t num_vertices() const = 0;
			virtual void impl_set_polygon() const = 0;
			virtual void impl_set_polygon_points() const = 0;
			virtual void impl_set_polygon_colors() const = 0;
		};
	}

	// ASSET
	class StaticPolygon : protected internal::PolygonSubmitter
	{
		mutable math::Triangulation triangulation;
		cmath::Polygon2D polygon;

	public:
		using internal::PolygonSubmitter::PolygonSubmitter;

		PolygonBatch* get_batch() const { return get_ref().get_batch(); }
		void set_batch(PolygonBatch* batch) { internal::PolygonSubmitter::set_batch(batch); }

		const cmath::Polygon2D& get_polygon() const { return polygon; }
		const std::vector<glm::vec2>& get_points() const { return polygon.points; }
		const std::vector<glm::vec4>& get_colors() const { return polygon.colors; }

		cmath::Polygon2D& set_polygon() { flag_all(); return polygon; }
		std::vector<glm::vec2>& set_points() { flag_points(); return polygon.points; }
		std::vector<glm::vec4>& set_colors() { flag_colors(); return polygon.colors; }

		void draw() const;

	private:
		void triangulate() const override;
		size_t num_vertices() const override;
		void impl_set_polygon() const override;
		void impl_set_polygon_points() const override;
		void impl_set_polygon_colors() const override;
	};

	class Polygonal : protected internal::PolygonSubmitter
	{
	public:
		Transformer2D transformer;

		using internal::PolygonSubmitter::PolygonSubmitter;
		virtual ~Polygonal() = default;

		PolygonBatch* get_batch() const { return get_ref().get_batch(); }
		void set_batch(PolygonBatch* batch) { internal::PolygonSubmitter::set_batch(batch); flag_all(); }

		const Transform2D& get_local() const { return transformer.get_local(); }
		Transform2D& set_local() { return transformer.set_local(); }

		void draw() const;

	protected:
		virtual void draw_triangulation(GLuint initial_vertex) const = 0;
	};

	class Polygon : public Polygonal
	{
		cmath::Polygon2D polygon;

	public:
		using Polygonal::Polygonal;

		const cmath::Polygon2D& get_polygon() const { return polygon; }
		const std::vector<glm::vec2>& get_points() const { return polygon.points; }
		const std::vector<glm::vec4>& get_colors() const { return polygon.colors; }

		cmath::Polygon2D& set_polygon() { flag_all(); return polygon; }
		std::vector<glm::vec2>& set_points() { flag_points(); return polygon.points; }
		std::vector<glm::vec4>& set_colors() { flag_colors(); return polygon.colors; }

		size_t num_vertices() const override;

	private:
		mutable math::Triangulation cache;

	protected:
		void impl_set_polygon() const override;
		void impl_set_polygon_points() const override;
		void impl_set_polygon_colors() const override;
		void triangulate() const override;
		void draw_triangulation(GLuint initial_vertex) const override;
	};

	class PolyComposite : public Polygonal
	{
		cmath::Polygon2DComposite composite;

	public:
		using Polygonal::Polygonal;

		const cmath::Polygon2DComposite& get_composite() const { return composite; }
		const cmath::TriangulatedPolygon2D& get_triangulated_polygon(size_t i) const { return composite[i]; }
		const math::Triangulation& get_triangulation(size_t i) const { return composite[i].triangulation; }
		const cmath::Polygon2D& get_polygon(size_t i) const { return composite[i].polygon; }
		const std::vector<glm::vec2>& get_points(size_t i) const { return composite[i].polygon.points; }
		const std::vector<glm::vec4>& get_colors(size_t i) const { return composite[i].polygon.colors; }

		cmath::Polygon2DComposite& set_composite() { flag_all(); return composite; }
		cmath::TriangulatedPolygon2D& set_triangulated_polygon(size_t i) { flag_all(); return composite[i]; }
		math::Triangulation& set_triangulation(size_t i) { flag_points(); return composite[i].triangulation; }
		cmath::Polygon2D& set_polygon(size_t i) { flag_all(); return composite[i].polygon; }
		std::vector<glm::vec2>& set_points(size_t i) { flag_points(); return composite[i].polygon.points; }
		std::vector<glm::vec4>& set_colors(size_t i) { flag_colors(); return composite[i].polygon.colors; }

		size_t num_vertices() const override;

	protected:
		void impl_set_polygon() const override;
		void impl_set_polygon_points() const override;
		void impl_set_polygon_colors() const override;
		void triangulate() const override;
		void draw_triangulation(GLuint initial_vertex) const override;
	};

	class NGon : public Polygonal
	{
		bool bordered = false;
		cmath::NGonBase base;

	public:
		using Polygonal::Polygonal;

		bool is_bordered() const { return bordered; }
		const cmath::NGonBase& get_base() const { return base; }
		const std::vector<glm::vec2>& get_points() const { return base.points; }
		const std::vector<glm::vec4>& get_fill_colors() const { return base.fill_colors; }
		const std::vector<glm::vec4>& get_border_colors() const { return base.border_colors; }
		float get_border_width() const { return base.border_width; }
		cmath::BorderPivot get_border_pivot() const { return base.border_pivot; }

		void set_bordered(bool b) { flag_all(); bordered = b; }
		cmath::NGonBase& set_base() { flag_all(); return base; }
		std::vector<glm::vec2>& set_points() { flag_points(); return base.points; }
		std::vector<glm::vec4>& set_fill_colors() { flag_colors(); return base.fill_colors; }
		std::vector<glm::vec4>& set_border_colors() { flag_colors(); return base.border_colors; }
		void set_border_width(float bw) { flag_points(); base.border_width = bw; }
		void set_border_pivot(cmath::BorderPivot pivot) { flag_points(); base.border_pivot = pivot; }

		size_t num_vertices() const override;

	private:
		mutable cmath::Polygon2DComposite cache;

	protected:
		void impl_set_polygon() const override;
		void impl_set_polygon_points() const override;
		void impl_set_polygon_colors() const override;
		void triangulate() const override;
		void draw_triangulation(GLuint initial_vertex) const override;
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
