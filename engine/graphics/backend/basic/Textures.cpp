#include "Textures.h"

#include "core/base/Approximate.h"
#include "core/base/Assert.h"
#include "core/base/Errors.h"

#include "core/context/rendering/Textures.h"

#include "core/util/IO.h"

#include "external/STB.h"

namespace oly::graphics
{
    Texture::Texture()
    {
        glGenTextures(1, &id_);
    }

    Texture::Texture(GLenum target)
    {
        glCreateTextures(target, 1, &id_);
    }

    Texture::Texture(Texture&& o) noexcept
        : id_(o.id_)
    {
        o.id_ = 0;
    }

    Texture::~Texture()
    {
        glDeleteTextures(1, &id_);
    }

    Texture& Texture::operator=(Texture&& o) noexcept
    {
        if (this != &o)
        {
            glDeleteTextures(1, &id_);
            id_ = o.id_;
            o.id_ = 0;
        }
        return *this;
    }

    Texture::operator GLuint () const
    {
        return id_;
    }

    void Texture::set_parameter(GLenum param, GLint value) const
    {
        glTextureParameteri(id_, param, value);
    }

    BindlessTexture::BindlessTexture()
    {
    }

    BindlessTexture::BindlessTexture(GLenum target)
        : tex_(target)
    {
    }

    BindlessTexture::BindlessTexture(Texture&& texture)
        : tex_(std::move(texture))
    {
    }

    BindlessTexture::BindlessTexture(BindlessTexture&& o) noexcept
        : tex_(std::move(o.tex_)), handle_(o.handle_)
    {
        o.handle_ = 0;
    }

    BindlessTexture::~BindlessTexture()
    {
        disuse_handle();
    }

    BindlessTexture& BindlessTexture::operator=(BindlessTexture&& o) noexcept
    {
        if (this != &o)
        {
            disuse_handle();
            tex_ = std::move(o.tex_);
            handle_ = o.handle_;
            o.handle_ = 0;
        }
        return *this;
    }

    BindlessTexture::operator GLuint () const
    {
        return tex_;
    }

    void BindlessTexture::set_handle()
    {
        disuse_handle();
        handle_ = glGetTextureHandleARB(tex_);
    }

    void BindlessTexture::set_handle(GLuint sampler)
    {
        disuse_handle();
        handle_ = glGetTextureSamplerHandleARB(tex_, sampler);
    }

    void BindlessTexture::set_and_use_handle()
    {
        set_handle();
        use_handle();
    }

    void BindlessTexture::set_and_use_handle(GLuint sampler)
    {
        set_handle(sampler);
        use_handle();
    }

    GLuint64 BindlessTexture::get_handle() const
    {
        return handle_;
    }

    void BindlessTexture::use_handle() const
    {
        if (handle_)
            glMakeTextureHandleResidentARB(handle_);
    }

    void BindlessTexture::disuse_handle() const
    {
        if (handle_)
            glMakeTextureHandleNonResidentARB(handle_);
    }

    const Texture& BindlessTexture::texture() const
    {
        return tex_;
    }

    ScopedPixelAlignment::ScopedPixelAlignment(int cpp)
        : cpp_(cpp)
    {
        if (cpp_ == 1 || cpp_ == 3)
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        else if (cpp_ == 2)
            glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
    }

    ScopedPixelAlignment::~ScopedPixelAlignment()
    {
        if (cpp_ != 4)
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    }

    glm::vec2 ImageDimensions::size() const
    {
        return { w, h };
    }

    GLenum tex::internal_format(int cpp)
    {
        switch (cpp)
        {
        case 4:
        default:
            return GL_RGBA8;

        case 1:
            return GL_R8;

        case 2:
            return GL_RG8;

        case 3:
            return GL_RGB8;
        }
    }

    GLenum tex::format(int cpp)
    {
        switch (cpp)
        {
        case 4:
        default:
            return GL_RGBA;

        case 1:
            return GL_RED;

        case 2:
            return GL_RG;

        case 3:
            return GL_RGB;
        }
    }

    GLint tex::mipmap_levels(int w, int h)
    {
        return GLint(glm::log2((float)glm::max(w, h))) + 1;
    }

    void tex::storage_2d(GLuint texture, ImageDimensions dim, GLsizei levels)
    {
        glTextureStorage2D(texture, levels, internal_format(dim.cpp), dim.w, dim.h);
    }

    void tex::storage_3d(GLuint texture, Image3DDimensions dim, GLsizei levels)
    {
        glTextureStorage3D(texture, levels, internal_format(dim.cpp), dim.w, dim.h, dim.d);
    }

    Image::Image(const detail::ResourcePath& file)
    {
        std::string f = file.get_absolute().string();
        buf_ = stbi_load(f.c_str(), &dim_.w, &dim_.h, &dim_.cpp, 0);
        if (!buf_)
            throw Error(ErrorCode::LoadImage);
    }

    Image::Image(unsigned char* buf, ImageDimensions dim)
        : buf_(buf), dim_(dim)
    {
    }

    Image::Image(Image&& o) noexcept
        : buf_(o.buf_), dim_(o.dim_)
    {
        o.buf_ = nullptr;
    }

    Image::~Image()
    {
        stbi_image_free(buf_);
    }

    Image& Image::operator=(Image&& o) noexcept
    {
        if (this != &o)
        {
            stbi_image_free(buf_);
            buf_ = o.buf_;
            dim_ = o.dim_;
            o.buf_ = nullptr;
        }
        return *this;
    }

    const unsigned char* Image::buf() const
    {
        return buf_;
    }
    
    unsigned char* Image::buf()
    {
        return buf_;
    }
    
    ImageDimensions Image::dim() const
    {
        return dim_;
    }

    void Image::delete_buffer()
    {
        stbi_image_free(buf_);
        buf_ = nullptr;
    }

    Texture load_texture_2d(const Image& image, bool generate_mipmaps)
    {
        Texture texture(GL_TEXTURE_2D);
        tex::image_2d(texture, image.buf(), image.dim(), generate_mipmaps);
        return texture;
    }

    Texture load_texture_2d(const detail::ResourcePath& file, ImageDimensions& dim, bool generate_mipmaps)
    {
        Image image(file);
        dim = image.dim();
        return load_texture_2d(image, generate_mipmaps);
    }

    BindlessTexture load_bindless_texture_2d(const Image& image, bool generate_mipmaps)
    {
        return BindlessTexture(load_texture_2d(image, generate_mipmaps));
    }

    BindlessTexture load_bindless_texture_2d(
        const detail::ResourcePath& file,
        ImageDimensions& dim,
        bool generate_mipmaps)
    {
        return BindlessTexture(load_texture_2d(file, dim, generate_mipmaps));
    }

    bool AnimDimensions::uniform() const
    {
        return frames_ >= 0;
    }

    GLuint AnimDimensions::frames() const
    {
        return uniform() ? static_cast<GLuint>(frames_) : static_cast<GLuint>(delays_.size());
    }

    float AnimDimensions::delay(unsigned int frame) const
    {
        if (frame >= frames())
            throw Error(ErrorCode::IndexOutOfRange);

        return uniform() ? delays_[0] : delays_[frame];
    }

    glm::vec2 AnimDimensions::size() const
    {
        return { w, h };
    }

    void AnimDimensions::set_delays(int* new_delays, unsigned int num_frames)
    {
        delays_.clear();
        frames_ = -1;
        if (num_frames == 0)
            return;

        delays_.resize(num_frames);
        bool single = true;
        const float delay = new_delays[0] * 0.01f;
        delays_[0] = delay;
        
        for (unsigned int i = 1; i < num_frames; ++i)
        {
            delays_[i] = new_delays[i] * 0.01f;
            if (std::abs(delays_[i] - delay) > 1.f) // animation delay epsilon is 1 ms
                single = false;
        }
        
        if (single)
        {
            delays_.resize(1);
            frames_ = num_frames;
        }
    }

    Anim::Anim(const detail::ResourcePath& file, SpritesheetOptions options)
    {
        if (file.extension_matches(".gif"))
        {
            auto full_content = io::read_file_uc(file);
            int* delays;
            int frames;

            buf_ = stbi_load_gif_from_memory(
                full_content.data(),
                static_cast<int>(full_content.size()),
                &delays,
                &dim_->w,
                &dim_->h,
                &frames,
                &dim_->cpp,
                0);
            
            dim_->set_delays(delays, frames);
            stbi_image_free(delays);
        }
        else
            parse_sprite_sheet(Image(file), options);
    }

    Anim::Anim(const NSVGAbstract& svg_abstract, float scale, SpritesheetOptions options)
    {
        parse_sprite_sheet(context::nsvg_context().rasterize(svg_abstract, scale), options);
    }

    Anim::Anim(Anim&& o) noexcept
        : buf_(o.buf_)
        , dim_(std::move(o.dim_))
    {
        o.buf_ = nullptr;
    }

    Anim::~Anim()
    {
        stbi_image_free(buf_);
    }

    Anim& Anim::operator=(Anim&& o) noexcept
    {
        if (this != &o)
        {
            stbi_image_free(buf_);
            buf_ = o.buf_;
            dim_ = std::move(o.dim_);
            o.buf_ = nullptr;
        }

        return *this;
    }

    const unsigned char* Anim::buf() const
    {
        return buf_;
    }
    
    unsigned char* Anim::buf()
    {
        return buf_;
    }
    
    SmartReference<AnimDimensions> Anim::dim() const
    {
        return dim_;
    }
    
    glm::vec2 Anim::size() const
    {
        return dim_->size();
    }

    void Anim::delete_buffer()
    {
        stbi_image_free(buf_);
        buf_ = nullptr;
    }

    void Anim::parse_sprite_sheet(const Image& image, SpritesheetOptions options)
    {
        dim_->delays_ = { options.delay };
        const auto idim = image.dim();
        dim_->cpp = idim.cpp;


        // Columns

        const GLuint xpx_offset = idim.w > 1 ? glm::min(options.col_offset_pixel, static_cast<GLuint>(idim.w)) : 0;
        const GLuint working_width = static_cast<GLuint>(idim.w) - xpx_offset;

        GLuint cols = options.col_type == detail::SpritesheetParamType::Index
            ? glm::clamp(options.col_value, 1u, working_width)
            : 1u;

        GLuint cell_width = options.col_type == detail::SpritesheetParamType::Pixel ? options.col_value : 0u;

        if (options.col_type == detail::SpritesheetParamType::Index || options.col_value == 0)
            cell_width = static_cast<int>(working_width / cols);
        else if (cell_width * cols > working_width)
            cols = static_cast<int>(working_width / cell_width);

        const GLuint col_offset = glm::min(options.col_offset_index, cols);
        cols -= col_offset;


        // Rows

        const GLuint ypx_offset = idim.h > 1 ? glm::min(options.row_offset_pixel, static_cast<GLuint>(idim.h)) : 0;
        const GLuint working_height = static_cast<GLuint>(idim.h) - ypx_offset;

        GLuint rows = options.row_type == detail::SpritesheetParamType::Index
            ? glm::clamp(options.row_value, 1u, working_height)
            : 1u;

        GLuint cell_height = options.row_type == detail::SpritesheetParamType::Pixel ? options.row_value : 0u;

        if (options.row_type == detail::SpritesheetParamType::Index || options.row_value == 0)
            cell_height = static_cast<int>(working_height / rows);
        else if (cell_height * rows > working_height)
            rows = static_cast<int>(working_height / cell_height);

        const GLuint row_offset = glm::min(options.row_offset_index, rows);
        rows -= row_offset;


        // Construction

        dim_->w = cell_width;
        dim_->h = cell_height;
        dim_->frames_ = rows * cols;

        const GLuint minor_stride = dim_->w * idim.cpp;
        const GLuint major_stride = minor_stride * cols;
        const GLuint image_major_stride = idim.w * idim.cpp;
        const GLuint minor_height = dim_->h;
        const GLuint major_height = minor_height * rows;
        const GLuint minor_area = minor_stride * minor_height;

        buf_ = new unsigned char[major_stride * major_height];

        const GLuint outer_count = options.row_major ? rows : cols;
        const GLuint inner_count = options.row_major ? cols : rows;

        const int i_start = options.row_up ? 0 : static_cast<int>(rows) - 1;
        const int i_step = options.row_up ? 1 : -1;

        GLuint k = 0;

        for (GLuint outer = 0; outer < outer_count; ++outer)
        {
            for (GLuint inner = 0; inner < inner_count; ++inner)
            {
                const GLuint i = options.row_major
                    ? (options.row_up ? outer : static_cast<GLuint>(i_start + static_cast<int>(outer) * i_step))
                    : (options.row_up ? inner : static_cast<GLuint>(i_start + static_cast<int>(inner) * i_step));

                const GLuint j = options.row_major ? inner : outer;

                const GLuint src_x = xpx_offset + (j + col_offset) * dim_->w;
                const GLuint src_y = ypx_offset + (i + row_offset) * dim_->h;

                for (GLuint rast_ = 0; rast_ < minor_height; ++rast_)
                {
                    memcpy(
                        buf_ + k * minor_area + rast_ * minor_stride,
                        image.buf() + src_x * dim_->cpp + (src_y + rast_) * image_major_stride,
                        minor_stride);
                }

                ++k;
            }
        }
    }

    Texture load_texture_2d_array(const Anim& anim, bool generate_mipmaps)
    {
        Texture texture(GL_TEXTURE_2D_ARRAY);
        const auto& dim = anim.dim();

        tex::storage_3d(
            texture,
            { .w = dim->w, .h = dim->h, .d = (int)dim->frames(), .cpp = dim->cpp },
            generate_mipmaps ? tex::mipmap_levels(dim->w, dim->h) : 1);
        
        for (GLuint i = 0; i < dim->frames(); ++i)
        {
            tex::subimage_3d(
                texture,
                anim.buf() + i * dim->w * dim->h * dim->cpp,
                { .z = (int)i, .w = dim->w, .h = dim->h, .d = 1 },
                dim->cpp);
        }
        
        if (generate_mipmaps)
            glGenerateTextureMipmap(texture);
        
        return texture;
    }

    Texture load_texture_2d_array(
        const detail::ResourcePath& file,
        SmartReference<AnimDimensions>& dim,
        SpritesheetOptions options,
        bool generate_mipmaps)
    {
        Anim anim(file, options);
        auto texture = load_texture_2d_array(anim, generate_mipmaps);
        dim = anim.dim();
        return texture;
    }

    BindlessTexture load_bindless_texture_2d_array(const Anim& anim, bool generate_mipmaps)
    {
        return BindlessTexture(load_texture_2d_array(anim, generate_mipmaps));
    }

    BindlessTexture load_bindless_texture_2d_array(
        const detail::ResourcePath& file,
        SmartReference<AnimDimensions>& dim,
        SpritesheetOptions options,
        bool generate_mipmaps)
    {
        return BindlessTexture(load_texture_2d_array(file, dim, options, generate_mipmaps));
    }

    AnimFrameFormat setup_anim_frame_format(const AnimDimensions& dim, float speed, GLuint starting_frame)
    {
        OLY_ASSERT(dim.uniform());
        return { starting_frame, dim.frames(), 0.0f, dim.delay() / speed };
    }

    AnimFrameFormat setup_anim_frame_format(
        const detail::ResourcePath& texture_file,
        float speed,
        GLuint starting_frame)
    {
        return setup_anim_frame_format(*context::get_anim_dimensions(texture_file), speed, starting_frame);
    }

    AnimFrameFormat setup_anim_frame_format_single(const AnimDimensions& dim, GLuint frame)
    {
        return { frame, dim.frames(), 0.0f, 0.0f };
    }

    AnimFrameFormat setup_anim_frame_format_single(const detail::ResourcePath& texture_file, GLuint frame)
    {
        return setup_anim_frame_format_single(*context::get_anim_dimensions(texture_file), frame);
    }

    NSVGAbstract::NSVGAbstract(const detail::ResourcePath& file, const char* units, float dpi)
    {
        std::string f = file.get_absolute().string();
        img_ = nsvgParseFromFile(f.c_str(), units, dpi);
        if (!img_)
            throw Error(ErrorCode::NsvgParsing);
    }

    NSVGAbstract::NSVGAbstract(NSVGAbstract&& o) noexcept
        : img_(o.img_)
    {
        o.img_ = nullptr;
    }

    NSVGAbstract::~NSVGAbstract()
    {
        nsvgDelete(img_);
    }

    NSVGAbstract& NSVGAbstract::operator=(NSVGAbstract&& o) noexcept
    {
        if (this != &o)
        {
            nsvgDelete(img_);
            img_ = o.img_;
            o.img_ = nullptr;
        }
        return *this;
    }

    float NSVGAbstract::width() const
    {
        return img_ ? img_->width : 0.0f;
    }

    float NSVGAbstract::height() const
    {
        return img_ ? img_->height : 0.0f;
    }

    NSVGContext::NSVGContext()
        : rast_(nsvgCreateRasterizer())
    {
    }

    NSVGContext::NSVGContext(NSVGContext&& o) noexcept
        : rast_(o.rast_)
    {
        o.rast_ = nullptr;
    }

    NSVGContext::~NSVGContext()
    {
        nsvgDeleteRasterizer(rast_);
    }

    NSVGContext& NSVGContext::operator=(NSVGContext&& o) noexcept
    {
        if (this != &o)
        {
            nsvgDeleteRasterizer(rast_);
            rast_ = o.rast_;
            o.rast_ = nullptr;
        }

        return *this;
    }

    Image NSVGContext::rasterize(const NSVGAbstract& abstract, float scale) const
    {
        unsigned char* buf;
        ImageDimensions dim;
        rasterize_unsafe(abstract, scale, buf, dim);
        return Image(buf, dim);
    }

    ImageRef NSVGContext::rasterize_res(const NSVGAbstract& abstract, float scale) const
    {
        unsigned char* buf;
        ImageDimensions dim;
        rasterize_unsafe(abstract, scale, buf, dim);
        return ImageRef(buf, dim);
    }

    void NSVGContext::rasterize_unsafe(
        const NSVGAbstract& abstract,
        float scale,
        unsigned char*& buf,
        ImageDimensions& dim) const
    {
        dim.w = std::max((int)(scale * abstract.width()), 1);
        dim.h = std::max((int)(scale * abstract.height()), 1);
        dim.cpp = 4;
        const int stride = dim.w * dim.cpp;
        buf = new unsigned char[stride * dim.h];
        nsvgRasterize(rast_, abstract.img_, 0.0f, 0.0f, scale, buf, dim.w, dim.h, stride);

        unsigned char* temp = new unsigned char[stride];
        for (int i = 0; i < dim.h / 2; ++i)
        {
            memcpy(temp, buf + i * stride, stride);
            memcpy(buf + i * stride, buf + (dim.h - 1 - i) * stride, stride);
            memcpy(buf + (dim.h - 1 - i) * stride, temp, stride);
        }
        delete[] temp;
    }

    Texture load_nsvg_texture_2d(
        const VectorImageRef& image,
        detail::SVGMipmapGenerationMode generate_mipmaps,
        const NSVGAbstract* abstract)
    {
        if (generate_mipmaps == detail::SVGMipmapGenerationMode::Manual && !abstract)
            generate_mipmaps = detail::SVGMipmapGenerationMode::Auto;

        Texture texture(GL_TEXTURE_2D);
        switch (generate_mipmaps)
        {
        case detail::SVGMipmapGenerationMode::Auto:
            tex::image_2d(texture, image.image->buf(), image.image->dim(), true);
            break;

        case detail::SVGMipmapGenerationMode::Off:
            tex::image_2d(texture, image.image->buf(), image.image->dim(), false);
            break;
        
        case detail::SVGMipmapGenerationMode::Manual:
        {
            const ImageDimensions dim = image.image->dim();
            const GLsizei levels = tex::mipmap_levels(dim.w, dim.h);
            tex::storage_2d(texture, dim, levels);
            float subscale = image.scale;
            
            for (GLsizei level = 0; level < levels; ++level)
            {
                Image subimage = context::nsvg_context().rasterize(*abstract, subscale);
                subscale *= 0.5f;

                tex::subimage_2d(
                    texture,
                    subimage.buf(),
                    { .w = glm::max(subimage.dim().w, 1), .h = glm::max(subimage.dim().h, 1) },
                    dim.cpp,
                    level);
            }

            break;
        }
        }
        
        return texture;
    }

    BindlessTexture load_bindless_nsvg_texture_2d(
        const VectorImageRef& image,
        detail::SVGMipmapGenerationMode generate_mipmaps,
        const NSVGAbstract* abstract)
    {
        return BindlessTexture(load_nsvg_texture_2d(image, generate_mipmaps, abstract));
    }
}
