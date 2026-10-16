#include "Framebuffers.h"

namespace oly::graphics
{
    Framebuffer internal::framebuffer_from_id(GLuint id)
    {
        return Framebuffer(id);
    }

    GLuint Framebuffer::ColorAttachment::max_color_attachments()
    {
        GLint v;
        glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &v);
        return v;
    }

    Framebuffer::Framebuffer(GLuint id)
        : id_(id)
    {
    }

    Framebuffer::Framebuffer()
    {
        glCreateFramebuffers(1, &id_);
    }

    Framebuffer::Framebuffer(Framebuffer&& other) noexcept
        : id_(other.id_)
    {
        other.id_ = 0;
    }

    Framebuffer::~Framebuffer()
    {
        glDeleteFramebuffers(1, &id_);
    }

    Framebuffer& Framebuffer::operator=(Framebuffer&& other) noexcept
    {
        if (this != &other)
        {
            glDeleteFramebuffers(1, &id_);
            id_ = other.id_;
            other.id_ = 0;
        }
        return *this;
    }

    Framebuffer::operator GLuint () const
    {
        return id_;
    }

    void Framebuffer::blit(
        const Framebuffer& read,
        const Framebuffer& draw,
        math::IRect2D src,
        math::IRect2D dst,
        BlitMask mask,
        BlitFilter filter)
    {
        glBlitNamedFramebuffer(
            read,
            draw,
            src.x1,
            src.y1,
            src.x2,
            src.y2,
            dst.x1,
            dst.y1,
            dst.x2,
            dst.y2,
            (GLenum)mask,
            (GLenum)filter);
    }

    void Framebuffer::blit_to_default(
        const Framebuffer& read,
        math::IRect2D src,
        math::IRect2D dst,
        BlitMask mask,
        BlitFilter filter)
    {
        glBlitNamedFramebuffer(
            read,
            0,
            src.x1,
            src.y1,
            src.x2,
            src.y2,
            dst.x1,
            dst.y1,
            dst.x2,
            dst.y2,
            (GLenum)mask,
            (GLenum)filter);
    }

    void Framebuffer::blit_from_default(
        const Framebuffer& draw,
        math::IRect2D src,
        math::IRect2D dst,
        BlitMask mask,
        BlitFilter filter)
    {
        glBlitNamedFramebuffer(
            0,
            draw,
            src.x1,
            src.y1,
            src.x2,
            src.y2,
            dst.x1,
            dst.y1,
            dst.x2,
            dst.y2,
            (GLenum)mask,
            (GLenum)filter);
    }

    void Framebuffer::unbind(Target target)
    {
        glBindFramebuffer((GLenum)target, 0);
    }
    
    void Framebuffer::bind(Target target) const
    {
        glBindFramebuffer((GLenum)target, id_);
    }

    void Framebuffer::attach_2d_texture(ColorAttachment a, GLuint texture, GLint level)
    {
        glNamedFramebufferTexture(id_, a, texture, level);
        if (std::find(color_attachments_.begin(), color_attachments_.end(), a) == color_attachments_.end())
            color_attachments_.push_back(a);
    }

    void Framebuffer::attach_2d_texture(NonColorAttachment a, GLuint texture, GLint level)
    {
        glNamedFramebufferTexture(id_, a, texture, level);
        if (a == NonColorAttachment::depth())
            depth_attached_ = true;
        else if (a == NonColorAttachment::stencil())
            stencil_attached_ = true;
        else if (a == NonColorAttachment::depth_stencil())
            depth_stencil_attached_ = true;
    }

    void Framebuffer::attach_3d_texture(ColorAttachment a, GLuint texture, GLint layer, GLint level)
    {
        glNamedFramebufferTextureLayer(id_, a, texture, level, layer);
        if (std::find(color_attachments_.begin(), color_attachments_.end(), a) == color_attachments_.end())
            color_attachments_.push_back(a);
    }

    void Framebuffer::attach_3d_texture(NonColorAttachment a, GLuint texture, GLint layer, GLint level)
    {
        glNamedFramebufferTextureLayer(id_, a, texture, level, layer);
        if (a == NonColorAttachment::depth())
            depth_attached_ = true;
        else if (a == NonColorAttachment::stencil())
            stencil_attached_ = true;
        else if (a == NonColorAttachment::depth_stencil())
            depth_stencil_attached_ = true;
    }

    void Framebuffer::detach_texture(ColorAttachment a)
    {
        glNamedFramebufferTexture(id_, a, 0, 0);
        auto it = std::find(color_attachments_.begin(), color_attachments_.end(), a);
        if (it != color_attachments_.end())
            color_attachments_.erase(it);
    }

    void Framebuffer::detach_texture(NonColorAttachment a)
    {
        glNamedFramebufferTexture(id_, a, 0, 0);
        if (a == NonColorAttachment::depth())
            depth_attached_ = false;
        else if (a == NonColorAttachment::stencil())
            stencil_attached_ = false;
        else if (a == NonColorAttachment::depth_stencil())
            depth_stencil_attached_ = false;
    }

    Framebuffer::Status Framebuffer::status(Target target) const
    {
        return (Status)glCheckNamedFramebufferStatus(id_, (GLenum)target);
    }

    void Framebuffer::draw_buffer(ColorAttachment color) const
    {
        glNamedFramebufferDrawBuffer(id_, color);
    }

    void Framebuffer::draw_buffers(const ColorAttachment* colors, GLsizei count) const
    {
        glNamedFramebufferDrawBuffers(id_, count, reinterpret_cast<const GLenum*>(colors));
    }

    void Framebuffer::draw_buffers() const
    {
        if (!color_attachments_.empty())
            glNamedFramebufferDrawBuffers(id_, (GLsizei)color_attachments_.size(), reinterpret_cast<const GLenum*>(color_attachments_.data()));
    }

    void Framebuffer::read_buffer(ColorAttachment color) const
    {
        glNamedFramebufferReadBuffer(id_, color);
    }

    std::vector<Framebuffer> framebuffer_block(size_t n)
    {
        GLuint* ids = new GLuint[n];
        glCreateFramebuffers((GLsizei)n, ids);
        std::vector<Framebuffer> fbs;
        fbs.reserve(n);
        for (size_t i = 0; i < n; ++i)
            fbs.push_back(internal::framebuffer_from_id(ids[i]));
        delete[] ids;
        return fbs;
    }
}
