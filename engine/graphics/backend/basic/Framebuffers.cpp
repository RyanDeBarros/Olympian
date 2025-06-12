#include "Framebuffers.h"

namespace oly::graphics
{
	Framebuffer::Framebuffer()
	{
		glGenFramebuffers(1, &id);
	}

	Framebuffer::Framebuffer(Framebuffer&& other) noexcept
		: id(other.id)
	{
		other.id = 0;
	}

	Framebuffer::~Framebuffer()
	{
		glDeleteFramebuffers(1, &id);
	}

	Framebuffer& Framebuffer::operator=(Framebuffer&& other) noexcept
	{
		if (this != &other)
		{
			glDeleteFramebuffers(1, &id);
			id = other.id;
			other.id = 0;
		}
		return *this;
	}

	void Framebuffer::attach_2d_texture(ColorAttachment a, GLuint texture, GLint level)
	{
		glNamedFramebufferTexture(id, a, texture, level);
		if (std::find(color_attachments.begin(), color_attachments.end(), a) == color_attachments.end())
			color_attachments.push_back(a);
	}

	void Framebuffer::attach_2d_texture(NonColorAttachment a, GLuint texture, GLint level)
	{
		glNamedFramebufferTexture(id, a, texture, level);
		if (a == NonColorAttachment::depth())
			depth_attached = true;
		else if (a == NonColorAttachment::stencil())
			stencil_attached = true;
		else if (a == NonColorAttachment::depth_stencil())
			depth_stencil_attached = true;
	}

	void Framebuffer::attach_3d_texture(ColorAttachment a, GLuint texture, GLint layer, GLint level)
	{
		glNamedFramebufferTextureLayer(id, a, texture, level, layer);
		if (std::find(color_attachments.begin(), color_attachments.end(), a) == color_attachments.end())
			color_attachments.push_back(a);
	}

	void Framebuffer::attach_3d_texture(NonColorAttachment a, GLuint texture, GLint layer, GLint level)
	{
		glNamedFramebufferTextureLayer(id, a, texture, level, layer);
		if (a == NonColorAttachment::depth())
			depth_attached = true;
		else if (a == NonColorAttachment::stencil())
			stencil_attached = true;
		else if (a == NonColorAttachment::depth_stencil())
			depth_stencil_attached = true;
	}

	void Framebuffer::detach_texture(ColorAttachment a)
	{
		glNamedFramebufferTexture(id, a, 0, 0);
		auto it = std::find(color_attachments.begin(), color_attachments.end(), a);
		if (it != color_attachments.end())
			color_attachments.erase(it);
	}

	void Framebuffer::detach_texture(NonColorAttachment a)
	{
		glNamedFramebufferTexture(id, a, 0, 0);
		if (a == NonColorAttachment::depth())
			depth_attached = false;
		else if (a == NonColorAttachment::stencil())
			stencil_attached = false;
		else if (a == NonColorAttachment::depth_stencil())
			depth_stencil_attached = false;
	}

	Framebuffer::Status Framebuffer::status(Target target) const
	{
		return (Status)glCheckNamedFramebufferStatus(id, (GLenum)target);
	}

	void Framebuffer::draw_buffer(ColorAttachment color) const
	{
		glNamedFramebufferDrawBuffer(id, color);
	}

	void Framebuffer::draw_buffers(const ColorAttachment* colors, GLsizei count) const
	{
		glNamedFramebufferDrawBuffers(id, count, reinterpret_cast<const GLenum*>(colors));
	}
	
	void Framebuffer::draw_buffers() const
	{
		if (!color_attachments.empty())
			glNamedFramebufferDrawBuffers(id, (GLsizei)color_attachments.size(), reinterpret_cast<const GLenum*>(color_attachments.data()));
	}

	void Framebuffer::read_buffer(ColorAttachment color) const
	{
		glNamedFramebufferReadBuffer(id, color);
	}

	void Framebuffer::blit(const Framebuffer& read, const Framebuffer& draw, math::IRect2D src, math::IRect2D dst, BlitMask mask, BlitFilter filter)
	{
		glBlitNamedFramebuffer(read, draw, src.x1, src.y1, src.x2, src.y2, dst.x1, dst.y1, dst.x2, dst.y2, (GLenum)mask, (GLenum)filter);
	}
	
	void Framebuffer::blit_to_default(const Framebuffer& read, math::IRect2D src, math::IRect2D dst, BlitMask mask, BlitFilter filter)
	{
		glBlitNamedFramebuffer(read, 0, src.x1, src.y1, src.x2, src.y2, dst.x1, dst.y1, dst.x2, dst.y2, (GLenum)mask, (GLenum)filter);
	}

	void Framebuffer::blit_from_default(const Framebuffer& draw, math::IRect2D src, math::IRect2D dst, BlitMask mask, BlitFilter filter)
	{
		glBlitNamedFramebuffer(0, draw, src.x1, src.y1, src.x2, src.y2, dst.x1, dst.y1, dst.x2, dst.y2, (GLenum)mask, (GLenum)filter);
	}

	std::vector<Framebuffer> framebuffer_block(const GLsizei n) // TODO do this instead of other block structs like GLBufferBlock
	{
		GLuint* ids = new GLuint[n];
		glGenFramebuffers(n, ids);
		std::vector<Framebuffer> fbs;
		fbs.reserve(n);
		for (GLsizei i = 0; i < n; ++i)
			fbs.push_back(std::move(Framebuffer(ids[i])));
		delete[] ids;
		return fbs;
	}
}
