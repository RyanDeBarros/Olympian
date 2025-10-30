#pragma once

#include "core/base/Constants.h"
#include "core/cmath/ColoredGeometry.h"
#include "core/containers/FreeSpaceTracker.h"
#include "core/containers/IDGenerator.h"
#include "core/types/Issuer.h"

#include "graphics/backend/specialized/ElementBuffers.h"
#include "graphics/backend/specialized/VertexBuffers.h"
#include "graphics/Tags.h"
#include "graphics/Camera.h"

namespace oly::rendering
{
	namespace internal
	{
		class PolygonReference;

		class PolygonBatch : public oly::internal::Issuer<PolygonBatch>
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
			Camera2DRef camera = REF_DEFAULT;

			PolygonBatch();
			PolygonBatch(const PolygonBatch&) = delete;
			PolygonBatch(PolygonBatch&&) = delete;

			void render() const;
			void render(const glm::mat3& projection) const;

		private:
			void set_primitive_points(Range<GLuint> vertex_range, const glm::vec2* points, GLuint count);
			void set_primitive_colors(Range<GLuint> vertex_range, const glm::vec4* colors, GLuint count);
			void set_polygon_transform(GLuint id, const glm::mat3& transform);
			const glm::mat3& get_polygon_transform(GLuint id);

			GLuint generate_id(GLuint vertices);
			void terminate_id(GLuint id);
			bool resize_range(GLuint& id, GLuint vertices);
			Range<GLuint> get_vertex_range(GLuint id) const;
			bool is_valid_id(GLuint id) const;

			StrictFreeSpaceTracker<GLuint> vertex_free_space;
			std::unordered_map<GLuint, Range<GLuint>> polygon_indexer;
			SoftIDGenerator<GLuint> id_generator;
			static const GLuint NULL_ID = GLuint(-1);
			void assert_valid_id(GLuint id) const;
		};
	}

	using PolygonBatch = PublicIssuer<internal::PolygonBatch>;

	namespace internal
	{
		class PolygonReference : public PublicIssuerHandle<PolygonBatch>
		{
			using Super = PublicIssuerHandle<PolygonBatch>;
			// ID refers to the index of a polygon in transform SSBO. It also indexes the set of ranges in the VBO block.
			mutable GLuint id = PolygonBatch::NULL_ID;
			
		public:
			PolygonReference(Unbatched = UNBATCHED);
			PolygonReference(rendering::PolygonBatch& batch);
			PolygonReference(const PolygonReference&);
			PolygonReference(PolygonReference&&) noexcept;
			~PolygonReference();
			PolygonReference& operator=(const PolygonReference&);
			PolygonReference& operator=(PolygonReference&&) noexcept;

			auto get_batch() const { return lock(); }
			void set_batch(Unbatched);
			void set_batch(rendering::PolygonBatch& batch);

			bool resize_range(GLuint vertices) const;
			Range<GLuint> get_vertex_range() const;

			void set_primitive_points(Range<GLuint> vertex_range, const glm::vec2* points, GLuint count) const;
			void set_primitive_colors(Range<GLuint> vertex_range, const glm::vec4* colors, GLuint count) const;
			void set_primitive_points(const glm::vec2* points, GLuint count) const;
			void set_primitive_colors(const glm::vec4* colors, GLuint count) const;
			void set_polygon_transform(const glm::mat3& transform) const;

			GLuint& draw_index() const;
		};

		class PolygonSubmitter
		{
			PolygonReference ref;
			mutable bool points = true;
			mutable bool colors = true;

		public:
			PolygonSubmitter(Unbatched = UNBATCHED);
			PolygonSubmitter(rendering::PolygonBatch& batch);
			PolygonSubmitter(const PolygonSubmitter&);
			PolygonSubmitter(PolygonSubmitter&&) noexcept = default;
			PolygonSubmitter& operator=(const PolygonSubmitter&);
			PolygonSubmitter& operator=(PolygonSubmitter&&) noexcept = default;
			virtual ~PolygonSubmitter() = default;

			auto get_batch() const { return ref.get_batch(); }
			void set_batch(Unbatched) { ref.set_batch(UNBATCHED); flag_all(); }
			void set_batch(rendering::PolygonBatch& batch) { ref.set_batch(batch); flag_all(); }

		protected:
			const internal::PolygonReference& get_ref() const { return ref; }

			void flag_points() { points = true; }
			void flag_colors() { colors = true; }
			void flag_all() { points = true; colors = true; }

			void submit_dirty() const;

			virtual void triangulate() const = 0;
			virtual GLuint num_vertices() const = 0;
			virtual void impl_set_polygon() const = 0;
			virtual void impl_set_polygon_points() const = 0;
			virtual void impl_set_polygon_colors() const = 0;
		};
	}

	// ASSET
	class StaticPolygon : public internal::PolygonSubmitter
	{
		mutable math::Triangulation triangulation;
		cmath::Polygon2D polygon;

	public:
		using internal::PolygonSubmitter::PolygonSubmitter;

		const cmath::Polygon2D& get_polygon() const { return polygon; }
		const std::vector<glm::vec2>& get_points() const { return polygon.points; }
		const std::vector<glm::vec4>& get_colors() const { return polygon.colors; }

		cmath::Polygon2D& set_polygon() { flag_all(); return polygon; }
		std::vector<glm::vec2>& set_points() { flag_points(); return polygon.points; }
		std::vector<glm::vec4>& set_colors() { flag_colors(); return polygon.colors; }

		void draw() const;

	private:
		void triangulate() const override;
		GLuint num_vertices() const override;
		void impl_set_polygon() const override;
		void impl_set_polygon_points() const override;
		void impl_set_polygon_colors() const override;
	};

	class Polygonal : public internal::PolygonSubmitter
	{
	public:
		Transformer2D transformer;

		using internal::PolygonSubmitter::PolygonSubmitter;
		virtual ~Polygonal() = default;

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

		GLuint num_vertices() const override;

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

		GLuint num_vertices() const override;

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

		GLuint num_vertices() const override;

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
