#include "Polygons.h"

#include "core/cmath/Triangulation.h"

#include "core/context/rendering/Rendering.h"

#include "core/math/Triangulation.h"

#include "core/util/Parser.h"

#include "graphics/resources/Shaders.h"

#include "graphics/shapes/Definitions.h"

#include "definitions/Keys.h"

#include <imp/util.hpp>

namespace oly::rendering
{
    internal::PolygonBatch::PolygonBatch()
        : ebo_(vao_)
        , vbo_block_(vao_)
        , vertex_free_space({ 0, nmax<GLuint>() })
    {
        shader_locations_.projection = glGetUniformLocation(graphics::internal_shaders::polygon_batch(), "uProjection");
        shader_locations_.invariant_projection = glGetUniformLocation(graphics::internal_shaders::polygon_batch(), "uInvariantProjection");

        vbo_block_.attributes[VBOSlot::Position] = graphics::VertexAttribute<graphics::VertexAttributeType::Float>{ .index = 0, .size = 2 };
        vbo_block_.attributes[VBOSlot::Color] = graphics::VertexAttribute<graphics::VertexAttributeType::Float>{ .index = 1, .size = 4 };
        vbo_block_.attributes[VBOSlot::Index] = graphics::VertexAttribute<graphics::VertexAttributeType::Int>{ .index = 2, .size = 1 };
        vbo_block_.setup();
    }

    void internal::PolygonBatch::render() const
    {
        if (ebo_.empty() || !camera)
            return;

        vbo_block_.pre_draw_all();
        transform_ssbo_.pre_draw();
        glBindVertexArray(vao_);
        glUseProgram(graphics::internal_shaders::polygon_batch());
        glUniformMatrix3fv(shader_locations_.projection, 1, GL_FALSE, glm::value_ptr(camera->projection_matrix()));
        glUniformMatrix3fv(shader_locations_.invariant_projection, 1, GL_FALSE, glm::value_ptr(camera->invariant_projection_matrix()));

        transform_ssbo_.buf.bind_ssbo_base(0);
        ebo_.render_elements(GL_TRIANGLES);
        vbo_block_.post_draw_all();
        transform_ssbo_.post_draw();
    }

    void internal::PolygonBatch::set_primitive_points(Range<GLuint> range, const glm::vec2* points, GLuint count)
    {
        count = std::min(range.length, count);
        for (GLuint v = 0; v < count; ++v)
            vbo_block_.set<VBOSlot::Position>(range.initial + v) = points[v];
    }

    void internal::PolygonBatch::set_primitive_colors(Range<GLuint> range, const glm::vec4* colors, GLuint count)
    {
        count = std::min(range.length, count);
        if (count == 1)
        {
            for (GLuint v = 0; v < range.length; ++v)
                vbo_block_.set<VBOSlot::Color>(range.initial + v) = colors[0];
        }
        else
        {
            for (GLuint v = 0; v < count; ++v)
                vbo_block_.set<VBOSlot::Color>(range.initial + v) = colors[v];
        }
    }

    void internal::PolygonBatch::set_polygon_transform(GLuint id, const glm::mat3& transform)
    {
        assert_valid_id(id);
        transform_ssbo_.set(id).transform = transform;
    }

    void internal::PolygonBatch::set_polygon_camera_invariant(GLuint id, bool camera_invariant_)
    {
        assert_valid_id(id);
        transform_ssbo_.set(id).camera_invariant = camera_invariant_;
    }

    const glm::mat3& internal::PolygonBatch::get_polygon_transform(GLuint id) const
    {
        assert_valid_id(id);
        return transform_ssbo_.get(id).transform;
    }

    bool internal::PolygonBatch::is_polygon_camera_invariant(GLuint id) const
    {
        assert_valid_id(id);
        return transform_ssbo_.get(id).camera_invariant;
    }

    GLuint internal::PolygonBatch::generate_id(GLuint vertices)
    {
        GLuint id = id_generator.gen();
        if (id == kNullId) [[unlikely]]
            throw Error(ErrorCode::StorageOverflow);

        Range<GLuint> vertex_range;
        OLY_ASSERT(vertex_free_space.next_free(vertices, vertex_range));
        vertex_free_space.reserve(vertex_range);
        polygon_indexer[id] = vertex_range;
        for (GLuint v = 0; v < vertex_range.length; ++v)
            vbo_block_.set<VBOSlot::Index>(vertex_range.initial + v) = id;
        return id;
    }

    void internal::PolygonBatch::terminate_id(GLuint id)
    {
        if (is_valid_id(id))
        {
            auto it = polygon_indexer.find(id);
            if (it != polygon_indexer.end())
            {
                vertex_free_space.release(it->second);
                polygon_indexer.erase(it);
            }
        }
    }

    bool internal::PolygonBatch::resize_range(GLuint& id, GLuint vertices)
    {
        if (is_valid_id(id))
        {
            if (vertices == get_vertex_range(id).length)
                return false;
            terminate_id(id);
        }
        id = generate_id(vertices);
        return true;
    }

    Range<GLuint> internal::PolygonBatch::get_vertex_range(GLuint id) const
    {
        assert_valid_id(id);
        return polygon_indexer.find(id)->second;
    }

    bool internal::PolygonBatch::is_valid_id(GLuint id) const
    {
        return id != kNullId && polygon_indexer.count(id);
    }

    void internal::PolygonBatch::assert_valid_id(GLuint id) const
    {
        if (!is_valid_id(id))
            throw Error(ErrorCode::InvalidID);
    }

    internal::PolygonReference::PolygonReference(Unbatched)
    {
    }

    internal::PolygonReference::PolygonReference(rendering::PolygonBatch& batch)
    {
        set_batch(batch);
    }

    internal::PolygonReference::PolygonReference(const PolygonReference& other)
        : Super(other)
    {
        if (auto batch = lock())
        {
            if (batch->is_valid_id(other.id_))
            {
                const GLuint num_vertices = batch->get_vertex_range(other.id_).length;
                const glm::mat3 transform = batch->get_polygon_transform(other.id_);
                bool camera_invariant_ = batch->is_polygon_camera_invariant(other.id_);
                id_ = batch->generate_id(num_vertices);
                batch->set_polygon_transform(id_, transform);
                batch->set_polygon_camera_invariant(id_, camera_invariant_);
            }
        }
    }

    internal::PolygonReference::PolygonReference(PolygonReference&& other) noexcept
        : Super(std::move(other))
        , id_(other.id_)
    {
        other.id_ = PolygonBatch::kNullId;
    }

    internal::PolygonReference::~PolygonReference()
    {
        if (auto batch = lock())
            batch->terminate_id(id_);
    }

    internal::PolygonReference& internal::PolygonReference::operator=(const PolygonReference& other)
    {
        if (this != &other)
            *this = dupl(other);
        return *this;
    }

    internal::PolygonReference& internal::PolygonReference::operator=(PolygonReference&& other) noexcept
    {
        if (this != &other)
        {
            if (auto batch = lock())
                batch->terminate_id(id_);
            Super::operator=(std::move(other));
            id_ = other.id_;
            other.id_ = PolygonBatch::kNullId;
        }
        return *this;
    }

    internal::PolygonReference::accessor internal::PolygonReference::get_batch() const
    {
        return lock();
    }

    void internal::PolygonReference::set_batch(Unbatched)
    {
        if (auto batch = lock())
            batch->terminate_id(id_);
        id_ = PolygonBatch::kNullId;
        reset();
    }

    void internal::PolygonReference::set_batch(rendering::PolygonBatch& new_batch)
    {
        if (auto batch = lock())
        {
            if (batch.get() == new_batch.address())
                return;

            if (batch->is_valid_id(id_))
            {
                const GLuint num_vertices = batch->get_vertex_range(id_).length;
                const glm::mat3 transform = batch->get_polygon_transform(id_);
                const bool camera_invariant_ = batch->is_polygon_camera_invariant(id_);
                batch->terminate_id(id_);
                reset(*new_batch);
                batch = lock();
                id_ = batch->generate_id(num_vertices);
                batch->set_polygon_transform(id_, transform);
                batch->set_polygon_camera_invariant(id_, camera_invariant_);
            }
            else
                reset(*new_batch);
        }
        else
            reset(*new_batch);
    }

    bool internal::PolygonReference::resize_range(GLuint vertices) const
    {
        if (auto batch = lock()) [[likely]]
        {
            const glm::mat3 transform = batch->is_valid_id(id_) ? batch->get_polygon_transform(id_) : 1.0f;
            const bool camera_invariant_ = batch->is_valid_id(id_) ? batch->is_polygon_camera_invariant(id_) : false;
            const bool resized = batch->resize_range(id_, vertices);
            if (resized)
            {
                batch->set_polygon_transform(id_, transform);
                batch->set_polygon_camera_invariant(id_, camera_invariant_);
            }
            return resized;
        }
        else
            throw Error(ErrorCode::NullPointer);
    }

    Range<GLuint> internal::PolygonReference::get_vertex_range() const
    {
        if (auto batch = lock()) [[likely]]
            return batch->get_vertex_range(id_);
        else
            throw Error(ErrorCode::NullPointer);
    }

    void internal::PolygonReference::set_primitive_points(Range<GLuint> vertex_range, const glm::vec2* points, GLuint count) const
    {
        if (auto batch = lock()) [[likely]]
            batch->set_primitive_points(vertex_range, points, count);
        else
            throw Error(ErrorCode::NullPointer);
    }

    void internal::PolygonReference::set_primitive_colors(Range<GLuint> vertex_range, const glm::vec4* colors, GLuint count) const
    {
        if (auto batch = lock()) [[likely]]
            batch->set_primitive_colors(vertex_range, colors, count);
        else
            throw Error(ErrorCode::NullPointer);
    }

    void internal::PolygonReference::set_primitive_points(const glm::vec2* points, GLuint count) const
    {
        if (auto batch = lock()) [[likely]]
            batch->set_primitive_points(batch->get_vertex_range(id_), points, count);
        else
            throw Error(ErrorCode::NullPointer);
    }

    void internal::PolygonReference::set_primitive_colors(const glm::vec4* colors, GLuint count) const
    {
        if (auto batch = lock()) [[likely]]
            batch->set_primitive_colors(batch->get_vertex_range(id_), colors, count);
        else
            throw Error(ErrorCode::NullPointer);
    }

    void internal::PolygonReference::set_polygon_transform(const glm::mat3& transform) const
    {
        if (auto batch = lock()) [[likely]]
            batch->set_polygon_transform(id_, transform);
        else
            throw Error(ErrorCode::NullPointer);
    }

    void internal::PolygonReference::set_camera_invariant(bool camera_invariant_) const
    {
        if (auto batch = lock()) [[likely]]
            batch->set_polygon_camera_invariant(id_, camera_invariant_);
        else
            throw Error(ErrorCode::NullPointer);
    }

    const glm::mat3& internal::PolygonReference::get_polygon_transform() const
    {
        if (auto batch = lock()) [[likely]]
            return batch->get_polygon_transform(id_);
        else
            throw Error(ErrorCode::NullPointer);
    }

    bool internal::PolygonReference::is_camera_invariant() const
    {
        if (auto batch = lock()) [[likely]]
            return batch->is_polygon_camera_invariant(id_);
        else
            throw Error(ErrorCode::NullPointer);
    }

    GLuint& internal::PolygonReference::draw_index() const
    {
        if (auto batch = lock()) [[likely]]
        {
            batch->assert_valid_id(id_);
            return batch->ebo_.draw_primitive()[0];
        }
        else
            throw Error(ErrorCode::NullPointer);
    }

    internal::PolygonSubmitter::PolygonSubmitter(Unbatched)
        : ref_(kUnbatched)
    {
    }

    internal::PolygonSubmitter::PolygonSubmitter(rendering::PolygonBatch& batch)
        : ref_(batch)
    {
    }

    internal::PolygonSubmitter::PolygonSubmitter(const PolygonSubmitter& other)
        : ref_(other.ref_)
    {
    }

    internal::PolygonSubmitter& internal::PolygonSubmitter::operator=(const PolygonSubmitter& other)
    {
        if (this != &other)
        {
            ref_ = other.ref_;
            flag_all();
        }
        return *this;
    }

    internal::PolygonReference::accessor internal::PolygonSubmitter::get_batch() const
    {
        return ref_.get_batch();
    }

    void internal::PolygonSubmitter::set_batch(Unbatched)
    {
        ref_.set_batch(kUnbatched);
        flag_all();
    }

    void internal::PolygonSubmitter::set_batch(rendering::PolygonBatch& batch)
    {
        ref_.set_batch(batch);
        flag_all();
    }

    void internal::PolygonSubmitter::set_camera_invariant(bool invariant) const
    {
        if (invariant)
            camera_invariant_ = CameraInvariantFlag(camera_invariant_ | CameraInvariantFlag::Value);
        else
            camera_invariant_ = CameraInvariantFlag(camera_invariant_ & ~CameraInvariantFlag::Value);

        camera_invariant_ = CameraInvariantFlag(camera_invariant_ | CameraInvariantFlag::Dirty);
    }

    bool internal::PolygonSubmitter::is_camera_invariant() const
    {
        return camera_invariant_ & CameraInvariantFlag::Value;
    }

    const internal::PolygonReference& internal::PolygonSubmitter::get_ref() const
    {
        return ref_;
    }

    void internal::PolygonSubmitter::flag_points()
    {
        points_ = true;
    }

    void internal::PolygonSubmitter::flag_colors()
    {
        colors_ = true;
    }

    void internal::PolygonSubmitter::flag_all()
    {
        points_ = true;
        colors_ = true;
    }

    void internal::PolygonSubmitter::submit_dirty() const
    {
        if (points_)
        {
            points_ = false;

            try
            {
                triangulate();
            }
            catch (const Error& e)
            {
                if (e.code == ErrorCode::Triangulation)
                    _OLY_ENGINE_LOG_WARNING("RENDERING") << "Error occurred during polygon triangulation." << LOG.nl;
                else
                    throw;
            }

            ref_.resize_range(num_vertices());

            if (colors_)
            {
                colors_ = false;
                impl_set_polygon();
            }
            else
                impl_set_polygon_points();
        }
        else if (colors_)
        {
            colors_ = false;
            impl_set_polygon_colors();
        }

        if (camera_invariant_ & CameraInvariantFlag::Dirty)
        {
            camera_invariant_ = CameraInvariantFlag(camera_invariant_ & ~CameraInvariantFlag::Dirty);
            ref_.set_camera_invariant(camera_invariant_ & CameraInvariantFlag::Value);
        }
    }

    void StaticPolygon::triangulate() const
    {
        triangulation_ = math::triangulate(polygon_.points);
    }

    GLuint StaticPolygon::num_vertices() const
    {
        return (GLuint)polygon_.points.size();
    }

    void StaticPolygon::impl_set_polygon() const
    {
        auto vertex_range = get_ref().get_vertex_range();
        get_ref().set_primitive_points(vertex_range, polygon_.points.data(), (GLuint)polygon_.points.size());
        get_ref().set_primitive_colors(vertex_range, polygon_.colors.data(), (GLuint)polygon_.colors.size());
    }

    void StaticPolygon::impl_set_polygon_points() const
    {
        get_ref().set_primitive_points(polygon_.points.data(), (GLuint)polygon_.points.size());
    }

    void StaticPolygon::impl_set_polygon_colors() const
    {
        get_ref().set_primitive_colors(polygon_.colors.data(), (GLuint)polygon_.colors.size());
    }

    const cmath::Polygon2D& StaticPolygon::get_polygon() const
    {
        return polygon_;
    }

    const std::vector<glm::vec2>& StaticPolygon::get_points() const
    {
        return polygon_.points;
    }

    const std::vector<glm::vec4>& StaticPolygon::get_colors() const
    {
        return polygon_.colors;
    }

    cmath::Polygon2D& StaticPolygon::set_polygon()
    {
        flag_all();
        return polygon_;
    }

    std::vector<glm::vec2>& StaticPolygon::set_points()
    {
        flag_points();
        return polygon_.points;
    }

    std::vector<glm::vec4>& StaticPolygon::set_colors()
    {
        flag_colors();
        return polygon_.colors;
    }

    void StaticPolygon::draw() const
    {
        submit_dirty();
        GLuint initial_vertex = get_ref().get_vertex_range().initial;
        for (size_t i = 0; i < triangulation_.size(); ++i)
        {
            get_ref().draw_index() = triangulation_[i][0] + initial_vertex;
            get_ref().draw_index() = triangulation_[i][1] + initial_vertex;
            get_ref().draw_index() = triangulation_[i][2] + initial_vertex;
        }
    }

    const Transform2D& Polygonal::get_local() const
    {
        return transformer.get_local();
    }
    
    Transform2D& Polygonal::set_local()
    {
        return transformer.set_local();
    }

    void Polygonal::draw() const
    {
        submit_dirty();
        if (transformer.flush())
            get_ref().set_polygon_transform(transformer.global());
        draw_triangulation(get_ref().get_vertex_range().initial);
    }

    GLuint Polygon::num_vertices() const
    {
        return (GLuint)polygon_.points.size();
    }

    void Polygon::impl_set_polygon() const
    {
        auto vertex_range = get_ref().get_vertex_range();
        get_ref().set_primitive_points(vertex_range, polygon_.points.data(), (GLuint)polygon_.points.size());
        get_ref().set_primitive_colors(vertex_range, polygon_.colors.data(), (GLuint)polygon_.colors.size());
    }

    void Polygon::impl_set_polygon_points() const
    {
        get_ref().set_primitive_points(polygon_.points.data(), (GLuint)polygon_.points.size());
    }

    void Polygon::impl_set_polygon_colors() const
    {
        get_ref().set_primitive_colors(polygon_.colors.data(), (GLuint)polygon_.colors.size());
    }

    void Polygon::triangulate() const
    {
        cache_ = math::triangulate(polygon_.points);
    }

    void Polygon::draw_triangulation(GLuint initial_vertex) const
    {
        for (size_t i = 0; i < cache_.size(); ++i)
        {
            get_ref().draw_index() = cache_[i][0] + initial_vertex;
            get_ref().draw_index() = cache_[i][1] + initial_vertex;
            get_ref().draw_index() = cache_[i][2] + initial_vertex;
        }
    }

    Polygon Polygon::load(TOMLNode node)
    {
        if (!node)
            return {};

        assets::Parser parser(node);

        Polygon polygon_;

        polygon_.transformer = Transformer2D::load(parser.field(detail::Key::Transformer));

        std::vector<glm::vec2> points;
        if (auto toml_points = parser.optional<TOMLArray>(detail::Key::PointsArray)())
        {
            size_t pt_idx = 0;
            for (auto& toml_point : *toml_points)
            {
                if (auto pt = assets::Parser((TOMLNode)toml_point, { "cannot parse point #", pt_idx }).optional<glm::vec2>(assets::NO_KEY)())
                    points.push_back(*pt);
                ++pt_idx;
            }
        }
        polygon_.set_points() = std::move(points);

        std::vector<glm::vec4> colors;
        if (auto toml_colors = parser.optional<TOMLArray>(detail::Key::ColorsArray)())
        {
            size_t color_idx = 0;
            for (auto& toml_color : *toml_colors)
            {
                if (auto col = assets::Parser((TOMLNode)toml_color, { "cannot parse color #", color_idx }).optional<glm::vec4>(assets::NO_KEY)())
                    colors.push_back(*col);
                ++color_idx;
            }
        }
        polygon_.set_colors() = std::move(colors);

        polygon_.set_camera_invariant(parser.defaulted(detail::Key::CameraInvariant)(false));

        return polygon_;
    }

    Polygon Polygon::load(TOMLNode node, const DebugTrace& trace)
    {
        auto scope = trace.scope("ASSETS", "oly::rendering::polygon_::load()");
        return load(node);
    }

    const cmath::Polygon2D& Polygon::get_polygon() const
    {
        return polygon_;
    }

    const std::vector<glm::vec2>& Polygon::get_points() const
    {
        return polygon_.points;
    }

    const std::vector<glm::vec4>& Polygon::get_colors() const
    {
        return polygon_.colors;
    }

    cmath::Polygon2D& Polygon::set_polygon()
    {
        flag_all();
        return polygon_;
    }

    std::vector<glm::vec2>& Polygon::set_points()
    {
        flag_points();
        return polygon_.points;
    }

    std::vector<glm::vec4>& Polygon::set_colors()
    {
        flag_colors();
        return polygon_.colors;
    }

    GLuint PolyComposite::num_vertices() const
    {
        GLuint vertices = 0;
        for (const auto& primitive : composite_)
            vertices += (GLuint)primitive.polygon.points.size();
        return vertices;
    }

    static void set_polygon_composite(const internal::PolygonReference& ref_, const cmath::Polygon2DComposite& composite_)
    {
        auto vertex_range = ref_.get_vertex_range();
        GLuint offset = 0;
        for (const cmath::TriangulatedPolygon2D& poly : composite_)
        {
            Range<GLuint> poly_range;
            poly_range.initial = vertex_range.initial + offset;
            if (poly_range.initial >= vertex_range.end())
                return;
            poly_range.length = std::min((GLuint)poly.polygon.points.size(), vertex_range.end() - poly_range.initial);
            ref_.set_primitive_points(poly_range, poly.polygon.points.data(), (GLuint)poly.polygon.points.size());
            ref_.set_primitive_colors(poly_range, poly.polygon.colors.data(), (GLuint)poly.polygon.colors.size());
            offset += (GLuint)poly.polygon.points.size();
        }
    }

    void PolyComposite::impl_set_polygon() const
    {
        set_polygon_composite(get_ref(), composite_);
    }

    static void set_polygon_points(const internal::PolygonReference& ref_, const cmath::Polygon2DComposite& composite_)
    {
        auto vertex_range = ref_.get_vertex_range();
        GLuint offset = 0;
        for (const cmath::TriangulatedPolygon2D& poly : composite_)
        {
            Range<GLuint> poly_range;
            poly_range.initial = vertex_range.initial + offset;
            if (poly_range.initial >= vertex_range.end())
                return;
            poly_range.length = std::min((GLuint)poly.polygon.points.size(), vertex_range.end() - poly_range.initial);
            ref_.set_primitive_points(poly_range, poly.polygon.points.data(), (GLuint)poly.polygon.points.size());
            offset += (GLuint)poly.polygon.points.size();
        }
    }

    void PolyComposite::impl_set_polygon_points() const
    {
        set_polygon_points(get_ref(), composite_);
    }

    static void set_polygon_colors(const internal::PolygonReference& ref_, const cmath::Polygon2DComposite& composite_)
    {
        auto vertex_range = ref_.get_vertex_range();
        GLuint offset = 0;
        for (const cmath::TriangulatedPolygon2D& poly : composite_)
        {
            Range<GLuint> poly_range;
            poly_range.initial = vertex_range.initial + offset;
            if (poly_range.initial >= vertex_range.end())
                return;
            poly_range.length = std::min((GLuint)poly.polygon.points.size(), vertex_range.end() - poly_range.initial);
            ref_.set_primitive_colors(poly_range, poly.polygon.colors.data(), (GLuint)poly.polygon.colors.size());
            offset += (GLuint)poly.polygon.points.size();
        }
    }

    void PolyComposite::impl_set_polygon_colors() const
    {
        set_polygon_colors(get_ref(), composite_);
    }

    void PolyComposite::triangulate() const
    {
    }

    void PolyComposite::draw_triangulation(GLuint initial_vertex) const
    {
        GLuint offset = 0;
        for (const auto& tp : composite_)
        {
            const auto& faces = tp.triangulation;
            for (size_t i = 0; i < faces.size(); ++i)
            {
                get_ref().draw_index() = faces[i][0] + initial_vertex + offset;
                get_ref().draw_index() = faces[i][1] + initial_vertex + offset;
                get_ref().draw_index() = faces[i][2] + initial_vertex + offset;
            }
            offset += (GLuint)tp.polygon.points.size();
        }
    }

    PolyComposite PolyComposite::load(TOMLNode node)
    {
        if (!node)
            return {};

        assets::Parser parser(node);

        PolyComposite polygon_;

        polygon_.transformer = Transformer2D::load(parser.field(detail::Key::Transformer));

        if (auto method = parser.optional<PolyCompositeMethod>(detail::Key::Method)())
        {
            switch (*method)
            {
            case PolyCompositeMethod::Ngon:
            {
                std::vector<glm::vec2> points;
                if (auto toml_points = parser.optional<TOMLArray>(detail::Key::PointsArray)())
                {
                    size_t pt_idx = 0;
                    for (auto& toml_point : *toml_points)
                    {
                        if (auto pt = assets::Parser((TOMLNode)toml_point, { "cannot parse point #", pt_idx }).optional<glm::vec2>(assets::NO_KEY)())
                            points.push_back(*pt);
                        ++pt_idx;
                    }
                }

                std::vector<glm::vec4> colors;
                if (auto toml_colors = parser.optional<TOMLArray>(detail::Key::ColorsArray)())
                {
                    size_t color_idx = 0;
                    for (auto& toml_color : *toml_colors)
                    {
                        if (auto col = assets::Parser((TOMLNode)toml_color, { "cannot parse color #", color_idx }).optional<glm::vec4>(assets::NO_KEY)())
                            colors.push_back(*col);
                        ++color_idx;
                    }
                }

                polygon_.set_composite() = cmath::Polygon2DComposite{ cmath::create_ngon(std::move(colors), std::move(points)) };
                break;
            }
            case PolyCompositeMethod::BorderedNgon:
            {
                cmath::NGonBase ngon_base;

                if (auto toml_points = parser.optional<TOMLArray>(detail::Key::PointsArray)())
                {
                    size_t pt_idx = 0;
                    for (auto& toml_point : *toml_points)
                    {
                        if (auto pt = assets::Parser((TOMLNode)toml_point, { "cannot parse point #", pt_idx }).optional<glm::vec2>(assets::NO_KEY)())
                            ngon_base.points.push_back(*pt);
                        ++pt_idx;
                    }
                }

                if (auto toml_fill_colors = parser.optional<TOMLArray>(detail::Key::FillColorsArray)())
                {
                    size_t color_idx = 0;
                    for (auto& toml_color : *toml_fill_colors)
                    {
                        if (auto col = assets::Parser((TOMLNode)toml_color, { "cannot parse fill color #", color_idx }).optional<glm::vec4>(assets::NO_KEY)())
                            ngon_base.fill_colors.push_back(*col);
                        ++color_idx;
                    }
                }

                if (auto toml_border_colors = parser.optional<TOMLArray>(detail::Key::BorderColorsArray)())
                {
                    size_t color_idx = 0;
                    for (auto& toml_color : *toml_border_colors)
                    {
                        if (auto col = assets::Parser((TOMLNode)toml_color, { "cannot parse border color #", color_idx }).optional<glm::vec4>(assets::NO_KEY)())
                            ngon_base.border_colors.push_back(*col);
                        ++color_idx;
                    }
                }

                parser.optional(detail::Key::BorderWidth)(ngon_base.border_width);

                if (!parser.optional<void, true>(detail::Key::BorderPivot)(ngon_base.border_pivot))
                    parser.optional(detail::Key::BorderPivot)(ngon_base.border_pivot.v);

                polygon_.set_composite() = cmath::create_bordered_ngon(std::move(ngon_base.fill_colors), std::move(ngon_base.border_colors),
                    ngon_base.border_width, ngon_base.border_pivot, std::move(ngon_base.points));
                break;
            }
            case PolyCompositeMethod::ConvexDecomposition:
            {
                std::vector<glm::vec2> points;

                if (auto toml_points = parser.optional<TOMLArray>(detail::Key::PointsArray)())
                {
                    size_t pt_idx = 0;
                    for (auto& toml_point : *toml_points)
                    {
                        if (auto pt = assets::Parser((TOMLNode)toml_point, { "cannot parse point #", pt_idx }).optional<glm::vec2>(assets::NO_KEY)())
                            points.push_back(*pt);
                        ++pt_idx;
                    }
                }

                polygon_.set_composite() = cmath::Decompose{}(std::move(points));
                break;
            }
            }
        }

        polygon_.set_camera_invariant(parser.defaulted(detail::Key::CameraInvariant)(false));

        return polygon_;
    }

    PolyComposite PolyComposite::load(TOMLNode node, const DebugTrace& trace)
    {
        auto scope = trace.scope("ASSETS", "oly::rendering::PolyComposite::load()");
        return load(node);
    }

    const cmath::Polygon2DComposite& PolyComposite::get_composite() const
    {
        return composite_;
    }

    const cmath::TriangulatedPolygon2D& PolyComposite::get_triangulated_polygon(size_t i) const
    {
        return composite_[i];
    }

    const math::Triangulation& PolyComposite::get_triangulation(size_t i) const
    {
        return composite_[i].triangulation;
    }

    const cmath::Polygon2D& PolyComposite::get_polygon(size_t i) const
    {
        return composite_[i].polygon;
    }

    const std::vector<glm::vec2>& PolyComposite::get_points(size_t i) const
    {
        return composite_[i].polygon.points;
    }

    const std::vector<glm::vec4>& PolyComposite::get_colors(size_t i) const
    {
        return composite_[i].polygon.colors;
    }

    cmath::Polygon2DComposite& PolyComposite::set_composite()
    {
        flag_all();
        return composite_;
    }

    cmath::TriangulatedPolygon2D& PolyComposite::set_triangulated_polygon(size_t i)
    {
        flag_all();
        return composite_[i];
    }

    math::Triangulation& PolyComposite::set_triangulation(size_t i)
    {
        flag_points();
        return composite_[i].triangulation;
    }

    cmath::Polygon2D& PolyComposite::set_polygon(size_t i)
    {
        flag_all();
        return composite_[i].polygon;
    }

    std::vector<glm::vec2>& PolyComposite::set_points(size_t i)
    {
        flag_points();
        return composite_[i].polygon.points;
    }

    std::vector<glm::vec4>& PolyComposite::set_colors(size_t i)
    {
        flag_colors();
        return composite_[i].polygon.colors;
    }

    GLuint NGon::num_vertices() const
    {
        GLuint vertices = 0;
        for (const auto& primitive : cache_)
            vertices += (GLuint)primitive.polygon.points.size();
        return vertices;
    }

    void NGon::impl_set_polygon() const
    {
        set_polygon_composite(get_ref(), cache_);
    }

    void NGon::impl_set_polygon_points() const
    {
        set_polygon_points(get_ref(), cache_);
    }

    void NGon::impl_set_polygon_colors() const
    {
        set_polygon_colors(get_ref(), cache_);
    }

    void NGon::triangulate() const
    {
        cache_ = bordered_ ? base_.bordered_composite() : base_.composite();
    }

    void NGon::draw_triangulation(GLuint initial_vertex) const
    {
        GLuint offset = 0;
        for (const auto& tp : cache_)
        {
            const auto& faces = tp.triangulation;
            for (size_t i = 0; i < faces.size(); ++i)
            {
                get_ref().draw_index() = faces[i][0] + initial_vertex + offset;
                get_ref().draw_index() = faces[i][1] + initial_vertex + offset;
                get_ref().draw_index() = faces[i][2] + initial_vertex + offset;
            }
            offset += (GLuint)tp.polygon.points.size();
        }
    }

    NGon NGon::load(TOMLNode node)
    {
        if (!node)
            return {};

        assets::Parser parser(node);

        NGon polygon_;

        polygon_.transformer = Transformer2D::load(parser.field(detail::Key::Transformer));

        cmath::NGonBase ngon_base;

        if (auto toml_points = parser.optional<TOMLArray>(detail::Key::PointsArray)())
        {
            size_t pt_idx = 0;
            for (auto& toml_point : *toml_points)
            {
                if (auto pt = assets::Parser((TOMLNode)toml_point, { "cannot parse point #", pt_idx }).optional<glm::vec2>(assets::NO_KEY)())
                    ngon_base.points.push_back(*pt);
                ++pt_idx;
            }
        }

        if (auto toml_fill_colors = parser.optional<TOMLArray>(detail::Key::FillColorsArray)())
        {
            size_t color_idx = 0;
            for (auto& toml_color : *toml_fill_colors)
            {
                if (auto col = assets::Parser((TOMLNode)toml_color, { "cannot parse fill color #", color_idx }).optional<glm::vec4>(assets::NO_KEY)())
                    ngon_base.fill_colors.push_back(*col);
                ++color_idx;
            }
        }

        if (auto toml_border_colors = parser.optional<TOMLArray>(detail::Key::BorderColorsArray)())
        {
            size_t color_idx = 0;
            for (auto& toml_color : *toml_border_colors)
            {
                if (auto col = assets::Parser((TOMLNode)toml_color, { "cannot parse border color #", color_idx }).optional<glm::vec4>(assets::NO_KEY)())
                    ngon_base.border_colors.push_back(*col);
                ++color_idx;
            }
        }

        if (auto bordered_ = parser.optional<bool>(detail::Key::Bordered)())
            polygon_.set_bordered(*bordered_);
        parser.optional(detail::Key::BorderWidth)(ngon_base.border_width);

        if (!parser.optional<void, true>(detail::Key::BorderPivot)(ngon_base.border_pivot))
            parser.optional(detail::Key::BorderPivot)(ngon_base.border_pivot.v);

        polygon_.set_base() = std::move(ngon_base);

        polygon_.set_camera_invariant(parser.defaulted(detail::Key::CameraInvariant)(false));

        return polygon_;
    }

    NGon NGon::load(TOMLNode node, const DebugTrace& trace)
    {
        auto scope = trace.scope("ASSETS", "oly::rendering::NGon::load()");
        return load(node);
    }

    bool NGon::is_bordered() const
    {
        return bordered_;
    }

    const cmath::NGonBase& NGon::get_base() const
    {
        return base_;
    }

    const std::vector<glm::vec2>& NGon::get_points() const
    {
        return base_.points;
    }

    const std::vector<glm::vec4>& NGon::get_fill_colors() const
    {
        return base_.fill_colors;
    }

    const std::vector<glm::vec4>& NGon::get_border_colors() const
    {
        return base_.border_colors;
    }

    float NGon::get_border_width() const
    {
        return base_.border_width;
    }

    cmath::BorderPivot NGon::get_border_pivot() const
    {
        return base_.border_pivot;
    }

    void NGon::set_bordered(bool b)
    {
        flag_all();
        bordered_ = b;
    }

    cmath::NGonBase& NGon::set_base()
    {
        flag_all();
        return base_;
    }

    std::vector<glm::vec2>& NGon::set_points()
    {
        flag_points();
        return base_.points;
    }

    std::vector<glm::vec4>& NGon::set_fill_colors()
    {
        flag_colors();
        return base_.fill_colors;
    }

    std::vector<glm::vec4>& NGon::set_border_colors()
    {
        flag_colors();
        return base_.border_colors;
    }

    void NGon::set_border_width(float bw)
    {
        flag_points();
        base_.border_width = bw;
    }

    void NGon::set_border_pivot(cmath::BorderPivot pivot)
    {
        flag_points();
        base_.border_pivot = pivot;
    }
}
