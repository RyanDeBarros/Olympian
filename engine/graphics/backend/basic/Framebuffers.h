#pragma once

#include "core/math/Shapes.h"

#include "graphics/backend/basic/Textures.h"

namespace oly::graphics
{
    class Framebuffer;

    namespace internal
    {
        extern Framebuffer framebuffer_from_id(GLuint id);
    }

    class Framebuffer
    {
        friend Framebuffer internal::framebuffer_from_id(GLuint id);

    public:
        enum class BlitMask
        {
            Color = GL_COLOR_BUFFER_BIT,
            Depth = GL_DEPTH_BUFFER_BIT,
            Stencil = GL_STENCIL_BUFFER_BIT
        };

        enum class BlitFilter
        {
            Nearest = GL_NEAREST,
            Linear = GL_LINEAR
        };

        // TODO imp utility for constrainted enum like this
        class NonColorAttachment
        {
            GLenum a_;

            explicit constexpr NonColorAttachment(GLenum a)
                : a_(a)
            {
            }

        public:
            static constexpr NonColorAttachment depth()
            {
                return NonColorAttachment(GL_DEPTH_ATTACHMENT);
            }

            static constexpr NonColorAttachment stencil()
            {
                return NonColorAttachment(GL_STENCIL_ATTACHMENT);
            }

            static constexpr NonColorAttachment depth_stencil()
            {
                return NonColorAttachment(GL_DEPTH_STENCIL_ATTACHMENT);
            }

            constexpr operator GLenum () const
            {
                return a_;
            }
        };

        class ColorAttachment
        {
            GLenum a_;

            explicit constexpr ColorAttachment(GLenum a)
                : a_(a)
            {
            }

        public:
            static constexpr ColorAttachment color(GLuint i)
            {
                return ColorAttachment(GL_COLOR_ATTACHMENT0 + i);
            }

            constexpr operator GLenum () const
            {
                return a_;
            }

            static GLuint max_color_attachments();
        };

    private:
        GLuint id_ = 0;
        bool depth_attached_ = false;
        bool stencil_attached_ = false;
        bool depth_stencil_attached_ = false;
        std::vector<ColorAttachment> color_attachments_;

    private:
        Framebuffer(GLuint id);

    public:
        Framebuffer();
        Framebuffer(const Framebuffer&) = delete;
        Framebuffer(Framebuffer&&) noexcept;
        ~Framebuffer();
        Framebuffer& operator=(Framebuffer&&) noexcept;

        operator GLuint () const;

        static void blit(
            const Framebuffer& read,
            const Framebuffer& draw,
            math::IRect2D src,
            math::IRect2D dst,
            BlitMask mask,
            BlitFilter filter);
        
        static void blit_to_default(
            const Framebuffer& read,
            math::IRect2D src,
            math::IRect2D dst,
            BlitMask mask,
            BlitFilter filter);
        
        static void blit_from_default(
            const Framebuffer& draw,
            math::IRect2D src,
            math::IRect2D dst,
            BlitMask mask,
            BlitFilter filter);

        enum class Target
        {
            RW = GL_FRAMEBUFFER,
            Draw = GL_DRAW_FRAMEBUFFER,
            Read = GL_READ_FRAMEBUFFER
        };

        static void unbind(Target target = Framebuffer::Target::RW);
        void bind(Target target = Framebuffer::Target::RW) const;

        void attach_2d_texture(ColorAttachment a, GLuint texture, GLint level = 0);
        void attach_2d_texture(NonColorAttachment a, GLuint texture, GLint level = 0);
        void attach_3d_texture(ColorAttachment a, GLuint texture, GLint layer, GLint level = 0);
        void attach_3d_texture(NonColorAttachment a, GLuint texture, GLint layer, GLint level = 0);
        void detach_texture(ColorAttachment a);
        void detach_texture(NonColorAttachment a);

        enum class Status
        {
            Complete = GL_FRAMEBUFFER_COMPLETE,
            Undefined = GL_FRAMEBUFFER_UNDEFINED,
            IncompleteAttachment = GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
            IncompleteMissingAttachment = GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT,
            IncompleteDrawBuffer = GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER,
            IncompleteReadBuffer = GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER,
            Unsupported = GL_FRAMEBUFFER_UNSUPPORTED,
            IncompleteMultisample = GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE,
            IncompleteLayerTargets = GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS
        };

        Status status(Target target = Target::RW) const;

        void draw_buffer(ColorAttachment a) const;
        void draw_buffers(const ColorAttachment* as, GLsizei count) const;
        void draw_buffers() const;

        void read_buffer(ColorAttachment color) const;
    };

    extern std::vector<Framebuffer> framebuffer_block(size_t n);

    namespace internal
    {
        template <size_t... Indices>
        inline std::array<Framebuffer, sizeof...(Indices)> framebuffer_block_impl(
            GLuint* ids,
            std::index_sequence<Indices...>)
        {
            return { framebuffer_from_id(ids[Indices])... };
        }
    }

    template <size_t N>
    inline std::array<Framebuffer, N> framebuffer_block()
    {
        GLuint ids[N];
        glCreateFramebuffers((GLsizei)N, ids);
        return internal::framebuffer_block_impl(ids, std::make_index_sequence<N>{});
    }
}
