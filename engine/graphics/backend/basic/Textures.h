#pragma once

#include "core/base/SmartReference.h"

#include "core/math/Shapes.h"

#include "graphics/backend/basic/Sampler.h"

#include "external/GLM.h"
#include "external/NSVG.h"

#include "assets/ResourcePath.h"

#include "definitions/enums/SpritesheetParamType.h"
#include "definitions/enums/SVGMipmapGenerationMode.h"

#include <imp/dependent_false.hpp>

#include <array>
#include <memory>
#include <string>
#include <vector>

namespace oly::graphics
{
    class Texture
    {
        GLuint id_ = 0;

    public:
        Texture();
        Texture(GLenum target);
        Texture(const Texture&) = delete;
        Texture(Texture&&) noexcept;
        ~Texture();

        Texture& operator=(Texture&&) noexcept;

        operator GLuint () const;

        void set_parameter(GLenum param, GLint value) const;
    };

    using TextureRef = SmartReference<Texture>;

    class BindlessTexture
    {
        Texture tex_;
        GLuint64 handle_ = 0;

    public:
        BindlessTexture();
        BindlessTexture(GLenum target);
        BindlessTexture(Texture&& texture);
        BindlessTexture(const BindlessTexture&) = delete;
        BindlessTexture(BindlessTexture&&) noexcept;
        ~BindlessTexture();

        BindlessTexture& operator=(BindlessTexture&&) noexcept;

        operator GLuint () const;

        void set_handle();
        void set_handle(GLuint sampler);

        void set_and_use_handle();
        void set_and_use_handle(GLuint sampler);

        GLuint64 get_handle() const;
        void use_handle() const;
        void disuse_handle() const;

        const Texture& texture() const;
    };

    using BindlessTextureRef = SmartReference<BindlessTexture>;

    class ScopedPixelAlignment
    {
        int cpp_;

    public:
        ScopedPixelAlignment(int cpp);
        ScopedPixelAlignment(const ScopedPixelAlignment&) = delete;
        ScopedPixelAlignment(ScopedPixelAlignment&&) = delete;
        ~ScopedPixelAlignment();
    };

    struct ImageDimensions
    {
        int w = 0;
        int h = 0;
        int cpp = 4;

        glm::vec2 size() const;

        template<typename Pixel = unsigned char>
        Pixel* pxnew() const { return new Pixel[w * h * cpp]; }
    };

    struct Image3DDimensions
    {
        int w = 0;
        int h = 0;
        int d = 0;
        int cpp = 4;
    };

    namespace tex
    {
        extern GLenum internal_format(int cpp);
        extern GLenum format(int cpp);
        extern GLint mipmap_levels(int w, int h);

        extern void storage_2d(GLuint texture, ImageDimensions dim, GLsizei levels = 1);

        template<typename Pixel>
        struct PixelDataType
        {
            static_assert(imp::dependent_false_v<Pixel>, "Unsupported pixel type");
        };

        template<>
        struct PixelDataType<unsigned char>
        {
            static constexpr GLenum v = GL_UNSIGNED_BYTE;
        };

        template<>
        struct PixelDataType<float>
        {
            static constexpr GLenum v = GL_FLOAT;
        };

        template<typename Pixel>
        constexpr GLenum kPixelDataType = PixelDataType<Pixel>::v;

        template<typename Pixel = unsigned char>
        inline void subimage_2d(GLuint texture, const Pixel* pixels, math::IArea2D subarea, int cpp, GLint level = 0)
        {
            ScopedPixelAlignment pixel_align(cpp);
            glTextureSubImage2D(
                texture,
                level,
                subarea.x,
                subarea.y,
                subarea.w,
                subarea.h,
                format(cpp),
                kPixelDataType<Pixel>,
                pixels);
        }

        template<typename Pixel = unsigned char>
        inline void image_2d(GLuint texture, const Pixel* pixels, ImageDimensions dim, bool mipmaps)
        {
            storage_2d(texture, dim, mipmaps ? mipmap_levels(dim.w, dim.h) : 1);
            subimage_2d(texture, pixels, { .w = dim.w, .h = dim.h }, dim.cpp);
            if (mipmaps)
                glGenerateTextureMipmap(texture);
        }

        extern void storage_3d(GLuint texture, Image3DDimensions dim, GLsizei levels = 1);

        template<typename Pixel = unsigned char>
        inline void subimage_3d(GLuint texture, const Pixel* pixels, math::IArea3D subarea, int cpp, GLint level = 0)
        {
            ScopedPixelAlignment pixel_align(cpp);
            glTextureSubImage3D(
                texture,
                level,
                subarea.x,
                subarea.y,
                subarea.z,
                subarea.w,
                subarea.h,
                subarea.d,
                format(cpp),
                kPixelDataType<Pixel>,
                pixels);
        }
    }

    class Image
    {
        unsigned char* buf_ = nullptr;
        ImageDimensions dim_;

    public:
        Image(const detail::ResourcePath& file);
        Image(unsigned char* buf, ImageDimensions dim);
        Image(const Image&) = delete;
        Image(Image&&) noexcept;
        ~Image();

        Image& operator=(Image&&) noexcept;

        const unsigned char* buf() const;
        unsigned char* buf();
        ImageDimensions dim() const;

        void delete_buffer();
    };

    using ImageRef = SmartReference<Image>;

    extern Texture load_texture_2d(const Image& image, bool generate_mipmaps = false);
    extern Texture load_texture_2d(
        const detail::ResourcePath& file,
        ImageDimensions& dim,
        bool generate_mipmaps = false);

    extern BindlessTexture load_bindless_texture_2d(const Image& image, bool generate_mipmaps = false);
    extern BindlessTexture load_bindless_texture_2d(
        const detail::ResourcePath& file,
        ImageDimensions& dim,
        bool generate_mipmaps = false);

    struct AnimDimensions
    {
        friend class Anim;

    public:
        int w = 0;
        int h = 0;
        int cpp = 4;

    private:
        int frames_ = -1;
        std::vector<float> delays_;

    public:
        bool uniform() const;
        GLuint frames() const;
        float delay(unsigned int frame = 0) const;

        glm::vec2 size() const;

    private:
        void set_delays(int* delays, unsigned int num_frames);
    };

    struct SpritesheetOptions
    {
        detail::SpritesheetParamType row_type = detail::SpritesheetParamType::Index;
        detail::SpritesheetParamType col_type = detail::SpritesheetParamType::Index;

        bool row_major = true;
        bool row_up = true;
        float delay = 0.1f;

        GLuint row_value = 1;
        GLuint col_value = 1;
        GLuint row_offset_index = 0;
        GLuint col_offset_index = 0;
        GLuint row_offset_pixel = 0;
        GLuint col_offset_pixel = 0;
    };

    class NSVGAbstract;

    class Anim
    {
        unsigned char* buf_ = nullptr;
        SmartReference<AnimDimensions> dim_ = kRefInit;

    public:
        Anim(const detail::ResourcePath& filepath, SpritesheetOptions options = {});
        Anim(const NSVGAbstract& svg_abstract, float scale, SpritesheetOptions options = {});
        Anim(const Anim&) = delete;
        Anim(Anim&&) noexcept;
        ~Anim();

        Anim& operator=(Anim&&) noexcept;

    public:
        const unsigned char* buf() const;
        unsigned char* buf();
        SmartReference<AnimDimensions> dim() const;
        glm::vec2 size() const;

        void delete_buffer();

    private:
        void parse_sprite_sheet(const Image& image, SpritesheetOptions options);
    };

    using AnimRef = SmartReference<Anim>;

    extern Texture load_texture_2d_array(const Anim& anim, bool generate_mipmaps = false);
    extern Texture load_texture_2d_array(
        const detail::ResourcePath& file,
        SmartReference<AnimDimensions>& dim,
        SpritesheetOptions options = {},
        bool generate_mipmaps = false);

    extern BindlessTexture load_bindless_texture_2d_array(const Anim& anim, bool generate_mipmaps = false);
    extern BindlessTexture load_bindless_texture_2d_array(
        const detail::ResourcePath& file,
        SmartReference<AnimDimensions>& dim,
        SpritesheetOptions options = {},
        bool generate_mipmaps = false);

    struct AnimFrameFormat
    {
        GLuint starting_frame = 0;
        GLuint num_frames = 0;
        float starting_time = 0.0f;
        float delay_seconds = 0.0f;

        bool operator==(const AnimFrameFormat&) const = default;
    };

    extern AnimFrameFormat setup_anim_frame_format(
        const AnimDimensions& dim,
        float speed = 1.0f,
        GLuint starting_frame = 0);

    extern AnimFrameFormat setup_anim_frame_format(
        const detail::ResourcePath& texture_file,
        float speed = 1.0f,
        GLuint starting_frame = 0);

    extern AnimFrameFormat setup_anim_frame_format_single(const AnimDimensions& dim, GLuint frame);
    extern AnimFrameFormat setup_anim_frame_format_single(const detail::ResourcePath& texture_file, GLuint frame);

    class NSVGAbstract
    {
        friend class NSVGContext;
        mutable NSVGimage* img_ = nullptr;

    public:
        NSVGAbstract(const detail::ResourcePath& file, const char* units = "px", float dpi = 96.0f);
        NSVGAbstract(const NSVGAbstract&) = delete;
        NSVGAbstract(NSVGAbstract&&) noexcept;
        ~NSVGAbstract();
        NSVGAbstract& operator=(NSVGAbstract&&) noexcept;

        float width() const;
        float height() const;
    };

    class NSVGContext
    {
        mutable NSVGrasterizer* rast_ = nullptr;

    public:
        NSVGContext();
        NSVGContext(const NSVGContext&) = delete;
        NSVGContext(NSVGContext&&) noexcept;
        ~NSVGContext();
        NSVGContext& operator=(NSVGContext&&) noexcept;

        Image rasterize(const NSVGAbstract& abstract, float scale) const;
        ImageRef rasterize_res(const NSVGAbstract& abstract, float scale) const;

    private:
        void rasterize_unsafe(
            const NSVGAbstract& abstract,
            float scale,
            unsigned char*& buf,
            ImageDimensions& dim) const;
    };

    struct VectorImageRef
    {
        ImageRef image;
        float scale = 1.0f;
    };
}

namespace oly::graphics
{
    extern Texture load_nsvg_texture_2d(
        const VectorImageRef& image,
        detail::SVGMipmapGenerationMode generate_mipmaps,
        const NSVGAbstract* abstract = nullptr);

    extern BindlessTexture load_bindless_nsvg_texture_2d(
        const VectorImageRef& image,
        detail::SVGMipmapGenerationMode generate_mipmaps,
        const NSVGAbstract* abstract = nullptr);
}
