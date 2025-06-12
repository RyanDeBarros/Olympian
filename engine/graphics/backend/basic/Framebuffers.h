#pragma once

#include "graphics/backend/basic/Textures.h"
#include "core/math/Shapes.h"

namespace oly::graphics
{
	class Framebuffer
	{
		GLuint id = 0;

		friend std::vector<Framebuffer> framebuffer_block(const GLsizei n);

		Framebuffer(GLuint id) : id(id) {}

	public:
		Framebuffer();
		Framebuffer(const Framebuffer&) = delete;
		Framebuffer(Framebuffer&&) noexcept;
		~Framebuffer();
		Framebuffer& operator=(Framebuffer&&) noexcept;

		operator GLuint () const { return id; }

		enum class Target
		{
			REG  = GL_FRAMEBUFFER,
			DRAW = GL_DRAW_FRAMEBUFFER,
			READ = GL_READ_FRAMEBUFFER
		};
		void bind(Target target = Framebuffer::Target::REG) const { glBindFramebuffer((GLenum)target, id); }
		static void unbind(Target target = Framebuffer::Target::REG) { glBindFramebuffer((GLenum)target, 0); }

		class NonColorAttachment
		{
			GLenum a;

			constexpr explicit NonColorAttachment(GLenum a) : a(a) {}

		public:
			static constexpr NonColorAttachment depth() { return NonColorAttachment(GL_DEPTH_ATTACHMENT); }
			static constexpr NonColorAttachment stencil() { return NonColorAttachment(GL_STENCIL_ATTACHMENT); }
			static constexpr NonColorAttachment depth_stencil() { return NonColorAttachment(GL_DEPTH_STENCIL_ATTACHMENT); }

			constexpr operator GLenum () const { return a; }
		};

		class ColorAttachment
		{
			GLenum a;

			constexpr explicit ColorAttachment(GLenum a) : a(a) {}

		public:
			static constexpr ColorAttachment color(GLuint i) { return ColorAttachment(GL_COLOR_ATTACHMENT0 + i); }
			static GLuint max_color_attachments() { GLint v; glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &v); return v; }

			constexpr operator GLenum () const { return a; }
		};

		void attach_2d_texture(ColorAttachment a, GLuint texture, GLint level = 0);
		void attach_2d_texture(NonColorAttachment a, GLuint texture, GLint level = 0);
		void attach_3d_texture(ColorAttachment a, GLuint texture, GLint layer, GLint level = 0);
		void attach_3d_texture(NonColorAttachment a, GLuint texture, GLint layer, GLint level = 0);
		void detach_texture(ColorAttachment a);
		void detach_texture(NonColorAttachment a);

		enum class Status
		{
			COMPLETE = GL_FRAMEBUFFER_COMPLETE,
			UNDEFINED = GL_FRAMEBUFFER_UNDEFINED,
			INCOMPLETE_ATTACHMENT = GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
			INCOMPLETE_MISSING_ATTACHMENT = GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT,
			INCOMPLETE_DRAW_BUFFER = GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER,
			INCOMPLETE_READ_BUFFER = GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER,
			UNSUPPORTED = GL_FRAMEBUFFER_UNSUPPORTED,
			INCOMPLETE_MULTISAMPLE = GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE,
			INCOMPLETE_LAYER_TARGETS = GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS
		};
		Status status(Target target = Target::REG) const;

		void draw_buffer(ColorAttachment a) const;
		void draw_buffers(const ColorAttachment* as, GLsizei count) const;
		void draw_buffers() const;

		void read_buffer(ColorAttachment color) const;

		enum class BlitMask
		{
			COLOR = GL_COLOR_BUFFER_BIT,
			DEPTH = GL_DEPTH_BUFFER_BIT,
			STENCIL = GL_STENCIL_BUFFER_BIT
		};
		enum class BlitFilter
		{
			NEAREST = GL_NEAREST,
			LINEAR = GL_LINEAR
		};
		static void blit(const Framebuffer& read, const Framebuffer& draw, math::IRect2D src, math::IRect2D dst, BlitMask mask, BlitFilter filter);
		static void blit_to_default(const Framebuffer& read, math::IRect2D src, math::IRect2D dst, BlitMask mask, BlitFilter filter);
		static void blit_from_default(const Framebuffer& draw, math::IRect2D src, math::IRect2D dst, BlitMask mask, BlitFilter filter);

	private:
		std::vector<ColorAttachment> color_attachments;
		bool depth_attached = false, stencil_attached = false, depth_stencil_attached = false;
	};
	
	extern std::vector<Framebuffer> framebuffer_block(const GLsizei n);

	// LATER Renderbuffer
}
