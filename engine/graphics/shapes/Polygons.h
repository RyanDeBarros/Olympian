#pragma once

#include "core/base/Constants.h"
#include "core/base/FreeSpaceTracker.h"

#include "core/cmath/ColoredGeometry.h"

#include "core/util/DebugTrace.h"

#include "graphics/Camera.h"
#include "graphics/Tags.h"

#include "graphics/backend/specialized/ElementBuffers.h"
#include "graphics/backend/specialized/VertexBuffers.h"

#include <imp/id_generator.hpp>

namespace oly::rendering
{
    namespace internal
    {
        class PolygonReference;

        class PolygonBatch
            : public imp::issuer<PolygonBatch>
        {
            friend class internal::PolygonReference;

        private:
            static const GLuint kNullId = GLuint(-1);

            graphics::VertexArray vao_;
            graphics::PersistentEBO<1> ebo_;

            struct
            {
                GLuint projection;
                GLuint invariant_projection;
            } shader_locations_;

            struct VBOSlot
            {
                enum
                {
                    Position,
                    Color,
                    Index
                };
            };

            graphics::PersistentVertexBufferBlock<glm::vec2, glm::vec4, GLuint> vbo_block_;

            struct MatInfo
            {
                glm::mat3 transform;
                GLubyte camera_invariant;
            };

            graphics::LazyPersistentGPUBuffer<MatInfo> transform_ssbo_;

            StrictFreeSpaceTracker<GLuint> vertex_free_space;
            std::unordered_map<GLuint, Range<GLuint>> polygon_indexer;
            imp::soft_id_generator<GLuint> id_generator;

        public:
            Camera2DRef camera = REF_DEFAULT;

        public:
            PolygonBatch();
            PolygonBatch(const PolygonBatch&) = delete;
            PolygonBatch(PolygonBatch&&) = delete;

        public:
            void render() const;

        private:
            void set_primitive_points(Range<GLuint> vertex_range, const glm::vec2* points, GLuint count);
            void set_primitive_colors(Range<GLuint> vertex_range, const glm::vec4* colors, GLuint count);
            void set_polygon_transform(GLuint id, const glm::mat3& transform);
            void set_polygon_camera_invariant(GLuint id, bool camera_invariant);
            const glm::mat3& get_polygon_transform(GLuint id) const;
            bool is_polygon_camera_invariant(GLuint id) const;

            GLuint generate_id(GLuint vertices);
            void terminate_id(GLuint id);
            bool resize_range(GLuint& id, GLuint vertices);
            Range<GLuint> get_vertex_range(GLuint id) const;
            bool is_valid_id(GLuint id) const;

            void assert_valid_id(GLuint id) const;
        };
    }

    using PolygonBatch = imp::issuer_instance<internal::PolygonBatch>;

    namespace internal
    {
        class PolygonReference
            : public imp::ticket<PolygonBatch>
        {
            using Super = imp::ticket<PolygonBatch>;
            // ID refers to the index of a polygon in transform SSBO. It also indexes the set of ranges in the VBO block.
            mutable GLuint id_ = PolygonBatch::kNullId;

        public:
            PolygonReference(Unbatched = kUnbatched);
            PolygonReference(rendering::PolygonBatch& batch);
            PolygonReference(const PolygonReference&);
            PolygonReference(PolygonReference&&) noexcept;
            ~PolygonReference();

            PolygonReference& operator=(const PolygonReference&);
            PolygonReference& operator=(PolygonReference&&) noexcept;

        public:
            accessor get_batch() const;
            void set_batch(Unbatched);
            void set_batch(rendering::PolygonBatch& batch);

            bool resize_range(GLuint vertices) const;
            Range<GLuint> get_vertex_range() const;

            void set_primitive_points(Range<GLuint> vertex_range, const glm::vec2* points, GLuint count) const;
            void set_primitive_colors(Range<GLuint> vertex_range, const glm::vec4* colors, GLuint count) const;
            void set_primitive_points(const glm::vec2* points, GLuint count) const;
            void set_primitive_colors(const glm::vec4* colors, GLuint count) const;
            void set_polygon_transform(const glm::mat3& transform) const;
            void set_camera_invariant(bool camera_invariant) const;

            const glm::mat3& get_polygon_transform() const;
            bool is_camera_invariant() const;

            GLuint& draw_index() const;
        };

        class PolygonSubmitter
        {
            PolygonReference ref_;
            mutable bool points_ = true;
            mutable bool colors_ = true;

            enum CameraInvariantFlag
            {
                Value = 1,
                Dirty = 2
            };

            mutable CameraInvariantFlag camera_invariant_ = CameraInvariantFlag(0b10);

        public:
            PolygonSubmitter(Unbatched = kUnbatched);
            PolygonSubmitter(rendering::PolygonBatch& batch);
            PolygonSubmitter(const PolygonSubmitter&);
            PolygonSubmitter(PolygonSubmitter&&) noexcept = default;
            virtual ~PolygonSubmitter() = default;

            PolygonSubmitter& operator=(const PolygonSubmitter&);
            PolygonSubmitter& operator=(PolygonSubmitter&&) noexcept = default;

        protected:
            virtual void triangulate() const = 0;
            virtual GLuint num_vertices() const = 0;
            virtual void impl_set_polygon() const = 0;
            virtual void impl_set_polygon_points() const = 0;
            virtual void impl_set_polygon_colors() const = 0;

        public:
            internal::PolygonReference::accessor get_batch() const;
            void set_batch(Unbatched);
            void set_batch(rendering::PolygonBatch& batch);

            void set_camera_invariant(bool camera_invariant) const;
            bool is_camera_invariant() const;

        protected:
            const internal::PolygonReference& get_ref() const;

            void flag_points();
            void flag_colors();
            void flag_all();

            void submit_dirty() const;
        };
    }

    // ASSET
    class StaticPolygon
        : public internal::PolygonSubmitter
    {
        mutable math::Triangulation triangulation_;
        cmath::Polygon2D polygon_;

    public:
        using internal::PolygonSubmitter::PolygonSubmitter;

    protected:
        void triangulate() const override;
        GLuint num_vertices() const override;
        void impl_set_polygon() const override;
        void impl_set_polygon_points() const override;
        void impl_set_polygon_colors() const override;

    public:
        const cmath::Polygon2D& get_polygon() const;
        const std::vector<glm::vec2>& get_points() const;
        const std::vector<glm::vec4>& get_colors() const;

        cmath::Polygon2D& set_polygon();
        std::vector<glm::vec2>& set_points();
        std::vector<glm::vec4>& set_colors();

        void draw() const;
    };

    class Polygonal
        : public internal::PolygonSubmitter
    {
    public:
        Transformer2D transformer;

    public:
        using internal::PolygonSubmitter::PolygonSubmitter;

    protected:
        virtual void draw_triangulation(GLuint initial_vertex) const = 0;

    public:
        const Transform2D& get_local() const;
        Transform2D& set_local();

        void draw() const;
    };

    class Polygon
        : public Polygonal
    {
        cmath::Polygon2D polygon_;
        mutable math::Triangulation cache_;

    public:
        using Polygonal::Polygonal;

    public:
        GLuint num_vertices() const override;

    protected:
        void impl_set_polygon() const override;
        void impl_set_polygon_points() const override;
        void impl_set_polygon_colors() const override;
        void triangulate() const override;
        void draw_triangulation(GLuint initial_vertex) const override;

    public:
        static Polygon load(TOMLNode node);
        static Polygon load(TOMLNode node, const DebugTrace& trace);

        const cmath::Polygon2D& get_polygon() const;
        const std::vector<glm::vec2>& get_points() const;
        const std::vector<glm::vec4>& get_colors() const;

        cmath::Polygon2D& set_polygon();
        std::vector<glm::vec2>& set_points();
        std::vector<glm::vec4>& set_colors();
    };

    class PolyComposite
        : public Polygonal
    {
        cmath::Polygon2DComposite composite_;

    public:
        using Polygonal::Polygonal;

    public:
        GLuint num_vertices() const override;

    protected:
        void impl_set_polygon() const override;
        void impl_set_polygon_points() const override;
        void impl_set_polygon_colors() const override;
        void triangulate() const override;
        void draw_triangulation(GLuint initial_vertex) const override;

    public:
        static PolyComposite load(TOMLNode node);
        static PolyComposite load(TOMLNode node, const DebugTrace& trace);

        const cmath::Polygon2DComposite& get_composite() const;
        const cmath::TriangulatedPolygon2D& get_triangulated_polygon(size_t i) const;
        const math::Triangulation& get_triangulation(size_t i) const;
        const cmath::Polygon2D& get_polygon(size_t i) const;
        const std::vector<glm::vec2>& get_points(size_t i) const;
        const std::vector<glm::vec4>& get_colors(size_t i) const;

        cmath::Polygon2DComposite& set_composite();
        cmath::TriangulatedPolygon2D& set_triangulated_polygon(size_t i);
        math::Triangulation& set_triangulation(size_t i);
        cmath::Polygon2D& set_polygon(size_t i);
        std::vector<glm::vec2>& set_points(size_t i);
        std::vector<glm::vec4>& set_colors(size_t i);

    };

    class NGon
        : public Polygonal
    {
        bool bordered_ = false;
        cmath::NGonBase base_;
        mutable cmath::Polygon2DComposite cache_;

    public:
        using Polygonal::Polygonal;

    public:
        GLuint num_vertices() const override;

    protected:
        void impl_set_polygon() const override;
        void impl_set_polygon_points() const override;
        void impl_set_polygon_colors() const override;
        void triangulate() const override;
        void draw_triangulation(GLuint initial_vertex) const override;

    public:
        static NGon load(TOMLNode node);
        static NGon load(TOMLNode node, const DebugTrace& trace);

        bool is_bordered() const;
        const cmath::NGonBase& get_base() const;
        const std::vector<glm::vec2>& get_points() const;
        const std::vector<glm::vec4>& get_fill_colors() const;
        const std::vector<glm::vec4>& get_border_colors() const;
        float get_border_width() const;
        cmath::BorderPivot get_border_pivot() const;

        void set_bordered(bool b);
        cmath::NGonBase& set_base();
        std::vector<glm::vec2>& set_points();
        std::vector<glm::vec4>& set_fill_colors();
        std::vector<glm::vec4>& set_border_colors();
        void set_border_width(float bw);
        void set_border_pivot(cmath::BorderPivot pivot);
    };
}

namespace oly
{
    OLY_SMART_POOL_BASE(rendering::Polygon, rendering::Polygonal);
    OLY_SMART_POOL_BASE(rendering::PolyComposite, rendering::Polygonal);
    OLY_SMART_POOL_BASE(rendering::NGon, rendering::Polygonal);
    
    namespace rendering
    {
        using PolygonalRef = SmartReference<Polygonal>;
        using PolygonRef = SmartReference<Polygon>;
        using PolyCompositeRef = SmartReference<PolyComposite>;
        using NGonRef = SmartReference<NGon>;
    }
}
