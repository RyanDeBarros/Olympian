#pragma once

#include "graphics/backend/specialized/PersistentBuffers.h"
#include "graphics/backend/specialized/VertexAttributes.h"

namespace oly::graphics
{
    template<typename... Structs>
    class PersistentVertexBufferBlock
    {
    public:
        template<size_t I>
        using StructAlias = typename LazyPersistentGPUBufferBlock<Structs...>::template StructAlias<I>;

        static constexpr size_t N = LazyPersistentGPUBufferBlock<Structs...>::N;

    private:
        LazyPersistentGPUBufferBlock<Structs...> buf_;
        GLuint vao_;

    public:
        std::array<VertexAttributeVariant, N> attributes;

        PersistentVertexBufferBlock(GLuint vao_, GLuint size = 0)
            : vao_(vao_)
            , buf_(size)
        {
        }

        PersistentVertexBufferBlock(GLuint vao_, const std::array<GLuint, N>& sizes)
            : vao_(vao_)
            , buf_(sizes) {
        }

    public:
        void setup()
        {
            glBindVertexArray(vao_);
            setup_impl(std::make_index_sequence<N>{});
            glBindVertexArray(0);
        }

        void set_vao(const VertexArray& vao_)
        {
            this->vao_ = vao_;
            setup();
        }

        template<size_t I>
        GLuint buffer() const
        {
            return buf_.buf.get_buffer<I>();
        }

        template<size_t I>
        GLuint size() const
        {
            return buf_.buf.get_size<I>();
        }

        template<size_t I>
        void pre_draw() const
        {
            buf_.pre_draw<I>();
        }

        void pre_draw_all() const
        {
            buf_.pre_draw_all();
        }

        template<size_t I>
        void post_draw() const
        {
            buf_.post_draw<I>();
        }

        void post_draw_all() const
        {
            buf_.post_draw_all();
        }

        template<size_t I>
        void grow()
        {
            buf_.grow<I>();
            setup_single<I>();
        }

        void grow_all() const
        {
            buf_.grow_all();
            setup();
        }

        template<size_t I>
        const StructAlias<I>& get(GLuint i) const
        {
            return buf_.get<I>(i);
        }

        template<size_t I>
        StructAlias<I>& set(GLuint i)
        {
            bool grown = false;
            StructAlias<I>& el = buf_.set<I>(i, &grown);
            if (grown)
                setup_single<I>();

            return el;
        }

        template<size_t I>
        const StructAlias<I>* get(GLuint offset, GLuint length) const
        {
            return buf_.get<I>(offset, length);
        }

        template<size_t I>
        StructAlias<I>* set(GLuint offset, GLuint length)
        {
            bool grown = false;
            StructAlias<I>* arr = buf_.set<I>(offset, length, &grown);
            if (grown)
                setup_single<I>();

            return arr;
        }

    private:
        template<size_t... Indices>
        void setup_impl(std::index_sequence<Indices...>)
        {
            (setup<Indices>(), ...);
        }

        template<size_t I>
        void setup()
        {
            static_assert(I < N);
            glBindBuffer(GL_ARRAY_BUFFER, buffer<I>());
            attributes[I].visit([](auto&& attribute) { attribute.setup(); });
        }

        template<size_t I>
        void setup_single()
        {
            static_assert(I < N);
            glBindVertexArray(vao_);
            setup<I>();
            glBindVertexArray(0);
        }
    };
}
