#pragma once

#include "core/base/Constants.h"

#include "core/util/DebugTrace.h"

#include "graphics/Camera.h"
#include "graphics/Tags.h"

#include "graphics/backend/basic/VertexArrays.h"

#include "graphics/backend/specialized/ElementBuffers.h"

#include <imp/id_generator.hpp>

namespace oly::rendering
{
    struct EllipseDimension
    {
        float rx = 0.0f;
        float ry = 0.0f;
        float border = 0.0f;
        float fill_exp = 0.0f;
        float border_exp = 0.0f;
        GLuint camera_invariant = 0;
    };

    struct EllipseColorGradient
    {
        glm::vec4 fill_inner = glm::vec4(1.0f);
        glm::vec4 fill_outer = glm::vec4(1.0f);
        glm::vec4 border_inner = glm::vec4(1.0f);
        glm::vec4 border_outer = glm::vec4(1.0f);

        bool is_uniform() const;
        void set_uniform(glm::vec4 color);
    };

    namespace internal
    {
        class EllipseReference;

        class EllipseBatch
            : public imp::issuer<EllipseBatch>
        {
            friend class EllipseReference;

        private:
            static const GLuint kNullId = GLuint(-1);

            graphics::VertexArray vao_;
            graphics::PersistentEBO<6> ebo_;

            struct SSBOSlot
            {
                enum
                {
                    Dimension,
                    Color,
                    Transform
                };
            };

            graphics::LazyPersistentGPUBufferBlock<EllipseDimension, EllipseColorGradient, glm::mat3> ssbo_block_;

            struct
            {
                GLuint projection;
                GLuint invariant_projection;
            } shader_locations_;

            imp::soft_id_generator<GLuint> id_generator_;

        public:
            Camera2DRef camera = kRefDefault;

        public:
            EllipseBatch();
            EllipseBatch(const EllipseBatch&) = delete;
            EllipseBatch(EllipseBatch&&) = delete;

        public:
            void render() const;

        private:
            static void assert_valid_id(GLuint id);

            GLuint generate_id();
            void erase_id(GLuint id);
        };
    }

    using EllipseBatch = imp::issuer_instance<internal::EllipseBatch>;

    namespace internal
    {
        class EllipseReference
            : public imp::ticket<EllipseBatch>
        {
            using Super = imp::ticket<EllipseBatch>;

            GLuint id_ = EllipseBatch::kNullId;

        public:
            EllipseReference(Unbatched = kUnbatched);
            EllipseReference(rendering::EllipseBatch& batch);
            EllipseReference(const EllipseReference&);
            EllipseReference(EllipseReference&&) noexcept;
            ~EllipseReference();

            EllipseReference& operator=(const EllipseReference&);
            EllipseReference& operator=(EllipseReference&&) noexcept;

        public:
            accessor get_batch() const;
            void set_batch(Unbatched);
            void set_batch(rendering::EllipseBatch& batch);

            EllipseDimension get_dimension() const;
            EllipseDimension& set_dimension() const;
            const EllipseColorGradient& get_color() const;
            EllipseColorGradient& set_color() const;
            void set_color(glm::vec4 color) const;
            const glm::mat3& get_transform() const;
            glm::mat3& set_transform() const;

            void draw() const;
        };
    }

    class StaticEllipse
    {
        internal::EllipseReference ref_;
        EllipseDimension dimension_;
        EllipseColorGradient color_;
        glm::mat3 transform_ = 1.0f;

    public:
        StaticEllipse(Unbatched = kUnbatched);
        StaticEllipse(EllipseBatch& batch);
        StaticEllipse(EllipseBatch& batch, float r, glm::vec4 color = glm::vec4(1.0f));
        StaticEllipse(EllipseBatch& batch, float rx, float ry, glm::vec4 color = glm::vec4(1.0f));
        StaticEllipse(const StaticEllipse&) = default;
        StaticEllipse(StaticEllipse&&) noexcept = default;

        StaticEllipse& operator=(const StaticEllipse&) = default;
        StaticEllipse& operator=(StaticEllipse&&) noexcept = default;

    public:
        void draw() const;

        internal::EllipseReference::accessor get_batch() const;
        void set_batch(Unbatched);
        void set_batch(EllipseBatch& batch);

        EllipseDimension get_dimension() const;
        void set_dimension(EllipseDimension dimension);
        const EllipseColorGradient& get_color() const;
        void set_color(const EllipseColorGradient& color);
        void set_color(glm::vec4 color);
        const glm::mat3& get_transform() const;
        void set_transform(const glm::mat3& local);

        math::Rect2D bounds() const;
        math::RotatedRect2D rotated_bounds() const;
    };

    class Ellipse
    {
        internal::EllipseReference ref_;
        Transformer2D transformer_;

    public:
        Ellipse(Unbatched = kUnbatched);
        Ellipse(EllipseBatch& batch);
        Ellipse(EllipseBatch& batch, float r, glm::vec4 color = glm::vec4(1.0f));
        Ellipse(EllipseBatch& batch, float rx, float ry, glm::vec4 color = glm::vec4(1.0f));
        Ellipse(const Ellipse&) = default;
        Ellipse(Ellipse&&) noexcept = default;

        Ellipse& operator=(const Ellipse&) = default;
        Ellipse& operator=(Ellipse&&) noexcept = default;

    public:
        static Ellipse load(TOMLNode node);
        static Ellipse load(TOMLNode node, const DebugTrace& trace);

        void draw() const;

        internal::EllipseReference::accessor get_batch() const;
        void set_batch(Unbatched);
        void set_batch(EllipseBatch& batch);

        EllipseDimension get_dimension() const;
        EllipseDimension& set_dimension();
        const EllipseColorGradient& get_color() const;
        EllipseColorGradient& set_color();
        void set_color(glm::vec4 color);

        const Transform2D& get_local() const;
        Transform2D& set_local();

        const Transformer2D& get_transformer() const;
        Transformer2D& set_transformer();
    };

    using EllipseRef = SmartReference<Ellipse>;
}
