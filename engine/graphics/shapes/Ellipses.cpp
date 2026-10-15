#include "Ellipses.h"

#include "core/context/rendering/Rendering.h"

#include "core/util/Parser.h"

#include "graphics/resources/Shaders.h"

#include "physics/collision/elements/OBB.h"

#include "definitions/Keys.h"

#include <imp/util.hpp>

namespace oly::rendering
{
    bool EllipseColorGradient::is_uniform() const
    {
        return fill_inner == fill_outer
            && fill_inner == border_inner
            && fill_inner == border_outer;
    }

    void EllipseColorGradient::set_uniform(glm::vec4 color)
    {
        fill_inner = color;
        fill_outer = color;
        border_inner = color;
        border_outer = color;
    }

    internal::EllipseBatch::EllipseBatch()
        : ebo_(vao_)
    {
        shader_locations_.projection = glGetUniformLocation(
            graphics::internal_shaders::ellipse_batch(),
            "uProjection");

        shader_locations_.invariant_projection = glGetUniformLocation(
            graphics::internal_shaders::ellipse_batch(),
            "uInvariantProjection");
    }

    void internal::EllipseBatch::render() const
    {
        if (ebo_.empty() || !camera)
            return;

        ssbo_block_.pre_draw_all();

        glBindVertexArray(vao_);
        glUseProgram(graphics::internal_shaders::ellipse_batch());

        glUniformMatrix3fv(
            shader_locations_.projection,
            1,
            GL_FALSE,
            glm::value_ptr(camera->projection_matrix()));
        
        glUniformMatrix3fv(
            shader_locations_.invariant_projection,
            1,
            GL_FALSE,
            glm::value_ptr(camera->invariant_projection_matrix()));

        ssbo_block_.buf.bind_ssbo_base<SSBOSlot::Dimension>(0);
        ssbo_block_.buf.bind_ssbo_base<SSBOSlot::Color>(1);
        ssbo_block_.buf.bind_ssbo_base<SSBOSlot::Transform>(2);
        ebo_.render_elements(GL_TRIANGLES);

        ssbo_block_.post_draw_all();
    }

    void internal::EllipseBatch::assert_valid_id(GLuint id)
    {
        if (id == kNullId) [[unlikely]]
            throw Error(ErrorCode::InvalidID);
    }

    GLuint internal::EllipseBatch::generate_id()
    {
        GLuint id = id_generator_.gen();
        if (id == kNullId)
            throw Error(ErrorCode::StorageOverflow);
        else
            return id;
    }

    void internal::EllipseBatch::erase_id(GLuint id)
    {
        if (id != kNullId) [[likely]]
            id_generator_.yield(id);
    }

    struct Attributes
    {
        EllipseDimension dimension = {};
        EllipseColorGradient color = {};
        glm::mat3 transform = 1.0f;
    };

    struct AttributesRef
    {
        EllipseDimension dimension = {};
        const EllipseColorGradient& color = {};
        const glm::mat3& transform = 1.0f;
    };

    static Attributes get_attributes(const internal::EllipseReference& ref_)
    {
        return {
            .dimension = ref_.get_dimension(),
            .color = ref_.get_color(),
            .transform = ref_.get_transform()
        };
    }

    static AttributesRef get_attributes_ref(const internal::EllipseReference& ref_)
    {
        return {
            .dimension = ref_.get_dimension(),
            .color = ref_.get_color(),
            .transform = ref_.get_transform()
        };
    }

    static void set_attributes(internal::EllipseReference& ref_, const Attributes& attr)
    {
        ref_.set_dimension() = attr.dimension;
        ref_.set_color() = attr.color;
        ref_.set_transform() = attr.transform;
    }

    static void set_attributes(internal::EllipseReference& ref_, const AttributesRef& attr)
    {
        ref_.set_dimension() = attr.dimension;
        ref_.set_color() = attr.color;
        ref_.set_transform() = attr.transform;
    }

    internal::EllipseReference::EllipseReference(Unbatched)
    {
    }

    internal::EllipseReference::EllipseReference(rendering::EllipseBatch& batch)
        : Super(batch->weak_from_this())
    {
        id_ = batch->generate_id();
        set_attributes(*this, Attributes{});
    }

    internal::EllipseReference::EllipseReference(const EllipseReference& other)
        : Super(other)
    {
        if (other.id_ != internal::EllipseBatch::kNullId)
        {
            if (auto batch = lock())
            {
                id_ = batch->generate_id();
                set_attributes(*this, get_attributes_ref(other));
            }
        }
    }

    internal::EllipseReference::EllipseReference(EllipseReference&& other) noexcept
        : Super(std::move(other)), id_(other.id_)
    {
        other.id_ = internal::EllipseBatch::kNullId;
    }

    internal::EllipseReference::~EllipseReference()
    {
        if (auto batch = lock())
            batch->erase_id(id_);
    }

    internal::EllipseReference& internal::EllipseReference::operator=(const EllipseReference& other)
    {
        if (this != &other)
            *this = dupl(other);
        return *this;
    }

    internal::EllipseReference& internal::EllipseReference::operator=(EllipseReference&& other) noexcept
    {
        if (this != &other)
        {
            if (auto batch = lock())
                batch->erase_id(id_);
            Super::operator=(std::move(other));
            id_ = other.id_;
            other.id_ = internal::EllipseBatch::kNullId;
        }
        return *this;
    }

    internal::EllipseReference::accessor internal::EllipseReference::get_batch() const
    {
        return lock();
    }

    void internal::EllipseReference::set_batch(Unbatched)
    {
        if (auto batch = lock())
            batch->erase_id(id_);
        id_ = internal::EllipseBatch::kNullId;
        reset();
    }

    void internal::EllipseReference::set_batch(rendering::EllipseBatch& new_batch)
    {
        if (auto batch = lock())
        {
            if (batch.get() == new_batch.address())
                return;

            const Attributes attr = id_ != internal::EllipseBatch::kNullId ? get_attributes(*this) : Attributes{};
            batch->erase_id(id_);
            reset(*new_batch);
            batch = lock();
            id_ = batch->generate_id();
            set_attributes(*this, attr);
        }
        else
        {
            reset(*new_batch);
            batch = lock();
            id_ = batch->generate_id();
            set_attributes(*this, Attributes{});
        }
    }

    EllipseDimension internal::EllipseReference::get_dimension() const
    {
        if (auto batch = lock()) [[likely]]
        {
            internal::EllipseBatch::assert_valid_id(id_);
            return batch->ssbo_block_.get<internal::EllipseBatch::SSBOSlot::Dimension>(id_);
        }
        else
            throw Error(ErrorCode::NullPointer);
    }

    EllipseDimension& internal::EllipseReference::set_dimension() const
    {
        if (auto batch = lock()) [[likely]]
        {
            internal::EllipseBatch::assert_valid_id(id_);
            return batch->ssbo_block_.set<internal::EllipseBatch::SSBOSlot::Dimension>(id_);
        }
        else
            throw Error(ErrorCode::NullPointer);
    }

    const EllipseColorGradient& internal::EllipseReference::get_color() const
    {
        if (auto batch = lock()) [[likely]]
        {
            internal::EllipseBatch::assert_valid_id(id_);
            return batch->ssbo_block_.get<internal::EllipseBatch::SSBOSlot::Color>(id_);
        }
        else
            throw Error(ErrorCode::NullPointer);
    }

    EllipseColorGradient& internal::EllipseReference::set_color() const
    {
        if (auto batch = lock()) [[likely]]
        {
            internal::EllipseBatch::assert_valid_id(id_);
            return batch->ssbo_block_.set<internal::EllipseBatch::SSBOSlot::Color>(id_);
        }
        else
            throw Error(ErrorCode::NullPointer);
    }

    void internal::EllipseReference::set_color(glm::vec4 color) const
    {
        set_color().set_uniform(color);
    }

    const glm::mat3& internal::EllipseReference::get_transform() const
    {
        if (auto batch = lock()) [[likely]]
        {
            internal::EllipseBatch::assert_valid_id(id_);
            return batch->ssbo_block_.get<internal::EllipseBatch::SSBOSlot::Transform>(id_);
        }
        else
            throw Error(ErrorCode::NullPointer);
    }

    glm::mat3& internal::EllipseReference::set_transform() const
    {
        if (auto batch = lock()) [[likely]]
        {
            internal::EllipseBatch::assert_valid_id(id_);
            return batch->ssbo_block_.set<internal::EllipseBatch::SSBOSlot::Transform>(id_);
        }
        else
            throw Error(ErrorCode::NullPointer);
    }

    void internal::EllipseReference::draw() const
    {
        if (auto batch = lock()) [[likely]]
        {
            internal::EllipseBatch::assert_valid_id(id_);
            graphics::quad_indices(batch->ebo_.draw_primitive().data(), id_);
        }
        else
            throw Error(ErrorCode::NullPointer);
    }

    StaticEllipse::StaticEllipse(Unbatched)
        : ref_(kUnbatched)
    {
    }
    
    StaticEllipse::StaticEllipse(EllipseBatch& batch)
        : ref_(batch)
    {
    }

    StaticEllipse::StaticEllipse(EllipseBatch& batch, float r, glm::vec4 color)
        : ref_(batch)
    {
        ref_.set_dimension().rx = r;
        ref_.set_dimension().ry = r;
        ref_.set_color(color);
    }

    StaticEllipse::StaticEllipse(EllipseBatch& batch, float rx, float ry, glm::vec4 color)
        : ref_(batch)
    {
        ref_.set_dimension().rx = rx;
        ref_.set_dimension().ry = ry;
        ref_.set_color(color);
    }

    void StaticEllipse::draw() const
    {
        ref_.draw();
    }

    internal::EllipseReference::accessor StaticEllipse::get_batch() const
    {
        return ref_.get_batch();
    }
    
    void StaticEllipse::set_batch(Unbatched)
    {
        ref_.set_batch(kUnbatched);
    }

    void StaticEllipse::set_batch(EllipseBatch& batch)
    {
        ref_.set_batch(batch);
        ref_.set_dimension() = dimension_;
        ref_.set_color() = color_;
        ref_.set_transform() = transform_;
    }

    EllipseDimension StaticEllipse::get_dimension() const
    {
        return dimension_;
    }

    void StaticEllipse::set_dimension(EllipseDimension dimension)
    {
        dimension_ = dimension;
        if (ref_.get_batch())
            ref_.set_dimension() = dimension;
    }

    const EllipseColorGradient& StaticEllipse::get_color() const
    {
        return color_;
    }

    void StaticEllipse::set_color(const EllipseColorGradient& color)
    {
        color_ = color;
        if (ref_.get_batch())
            ref_.set_color() = color;
    }

    void StaticEllipse::set_color(glm::vec4 color)
    {
        color_.set_uniform(color);
        if (ref_.get_batch())
            ref_.set_color(color);
    }

    const glm::mat3& StaticEllipse::get_transform() const
    {
        return transform_;
    }

    void StaticEllipse::set_transform(const glm::mat3& transform)
    {
        transform_ = transform;
        if (ref_.get_batch())
            ref_.set_transform() = transform;
    }

    math::Rect2D StaticEllipse::bounds() const
    {
        const EllipseDimension d = ref_.get_dimension();
        std::array<glm::vec2, 4> pts = math::Rect2D{ .x1 = -d.rx, .x2 = d.rx, .y1 = -d.ry, .y2 = d.ry }.uvs();

        for (size_t i = 0; i < 4; ++i)
            pts[i] = transform_point(transform_, pts[i]);

        math::Rect2D b{ .x1 = nmax<float>(), .x2 = -nmax<float>(), .y1 = nmax<float>(), .y2 = -nmax<float>() };
        for (size_t i = 0; i < 4; ++i)
        {
            b.x1 = imp::min(b.x1, pts[i].x);
            b.x2 = imp::max(b.x2, pts[i].x);
            b.y1 = imp::min(b.y1, pts[i].y);
            b.y2 = imp::max(b.y2, pts[i].y);
        }
        return b;
    }

    math::RotatedRect2D StaticEllipse::rotated_bounds() const
    {
        const EllipseDimension d = ref_.get_dimension();
        std::array<glm::vec2, 4> pts = math::Rect2D{ .x1 = -d.rx, .x2 = d.rx, .y1 = -d.ry, .y2 = d.ry }.uvs();

        for (size_t i = 0; i < 4; ++i)
            pts[i] = transform_point(transform_, pts[i]);

        return col2d::OBB::fast_wrap(pts.data(), 4).rect();
    }

    Ellipse::Ellipse(Unbatched)
        : ref_(kUnbatched)
    {
    }

    Ellipse::Ellipse(EllipseBatch& batch)
        : ref_(batch)
    {
    }

    Ellipse::Ellipse(EllipseBatch& batch, float r, glm::vec4 color)
        : ref_(batch)
    {
        ref_.set_dimension().rx = r;
        ref_.set_dimension().ry = r;
        ref_.set_color(color);
    }

    Ellipse::Ellipse(EllipseBatch& batch, float rx, float ry, glm::vec4 color)
        : ref_(batch)
    {
        ref_.set_dimension().rx = rx;
        ref_.set_dimension().ry = ry;
        ref_.set_color(color);
    }

    Ellipse Ellipse::load(TOMLNode node)
    {
        if (!node)
            return {};

        assets::Parser parser(node);

        rendering::Ellipse ellipse;
        ellipse.set_transformer() = Transformer2D::load(parser.field(detail::Key::Transformer));

        auto& color = ellipse.set_color();
        parser.optional(detail::Key::BorderInnerColor)(color.border_inner);
        parser.optional(detail::Key::BorderOuterColor)(color.border_outer);
        parser.optional(detail::Key::FillInnerColor)(color.fill_inner);
        parser.optional(detail::Key::FillOuterColor)(color.fill_outer);

        auto& dimension = ellipse.set_dimension();
        parser.optional(detail::Key::Border)(dimension.border);
        parser.optional(detail::Key::BorderExponent)(dimension.border_exp);
        parser.optional(detail::Key::FillExponent)(dimension.fill_exp);
        parser.optional(detail::Key::RadiusX)(dimension.rx);
        parser.optional(detail::Key::RadiusY)(dimension.ry);
        dimension.camera_invariant = parser.defaulted(detail::Key::CameraInvariant)(false);

        return ellipse;
    }

    Ellipse Ellipse::load(TOMLNode node, const DebugTrace& trace)
    {
        auto scope = trace.scope("ASSETS", "oly::rendering::Ellipse::load()");
        return load(node);
    }

    void Ellipse::draw() const
    {
        if (transformer_.flush())
            ref_.set_transform() = transformer_.global();

        ref_.draw();
    }

    internal::EllipseReference::accessor Ellipse::get_batch() const
    {
        return ref_.get_batch();
    }

    void Ellipse::set_batch(Unbatched)
    {
        ref_.set_batch(kUnbatched);
    }

    void Ellipse::set_batch(EllipseBatch& batch)
    {
        ref_.set_batch(batch);
    }

    EllipseDimension Ellipse::get_dimension() const
    {
        return ref_.get_dimension();
    }

    EllipseDimension& Ellipse::set_dimension()
    {
        return ref_.set_dimension();
    }

    const EllipseColorGradient& Ellipse::get_color() const
    {
        return ref_.get_color();
    }

    EllipseColorGradient& Ellipse::set_color()
    {
        return ref_.set_color();
    }

    void Ellipse::set_color(glm::vec4 color)
    {
        ref_.set_color(color);
    }

    const Transform2D& Ellipse::get_local() const
    {
        return transformer_.get_local();
    }

    Transform2D& Ellipse::set_local()
    {
        return transformer_.set_local();
    }

    const Transformer2D& Ellipse::get_transformer() const
    {
        return transformer_;
    }

    Transformer2D& Ellipse::set_transformer()
    {
        return transformer_;
    }
}
