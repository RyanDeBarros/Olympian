#pragma once

#include "core/base/Errors.h"
#include "core/base/Ranges.h"

#include "core/util/Logger.h"

#include "graphics/backend/basic/Buffers.h"
#include "graphics/backend/basic/FenceSync.h"

namespace oly::graphics
{
    namespace internal
    {
        struct PersistentOptions
        {
            GLuint64 timeout_ns = 10'000'000; // 10ms
            GLuint max_timeout_tries = 100; // 1s total
            float grow_multiplier = 1.8f;
        };
    }

    template<
        typename Struct,
        internal::PersistentOptions Options = internal::PersistentOptions{}>
    class PersistentGPUBuffer
    {
        GLBuffer buf_;
        mutable bool accessible_ = true;
        GLuint size_ = 0;
        void* data_ = nullptr;

    public:
        using StructAlias = Struct;

        PersistentGPUBuffer(GLuint size = 0)
            : size_(size)
        {
            if (size_)
            {
                glNamedBufferStorage(
                    buf_,
                    static_cast<GLsizeiptr>(size_ * sizeof(Struct)),
                    nullptr,
                    GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);

                data_ = glMapNamedBufferRange(
                    buf_,
                    0,
                    static_cast<GLsizeiptr>(size_ * sizeof(Struct)),
                    GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);
            }
        }

        PersistentGPUBuffer(const PersistentGPUBuffer&) = delete;
        PersistentGPUBuffer(PersistentGPUBuffer&&) = delete;

        GLuint get_size() const
        {
            return size_;
        }

        GLuint get_buffer() const
        {
            return buf_;
        }

        bool is_accessible() const
        {
            return accessible_;
        }

        const Struct& operator[](GLuint i) const
        {
            if (!accessible_)
                throw Error(ErrorCode::Inaccessible);

            if (i >= size_)
                throw Error(ErrorCode::IndexOutOfRange);
            
            return reinterpret_cast<const Struct*>(data_)[i];
        }

        Struct& operator[](GLuint i)
        {
            if (!accessible_)
                throw Error(ErrorCode::Inaccessible);

            if (i >= size_)
                throw Error(ErrorCode::IndexOutOfRange);
            
            return reinterpret_cast<Struct*>(data_)[i];
        }

        const Struct* arr(GLuint offset, GLuint length) const
        {
            if (!accessible_)
                throw Error(ErrorCode::Inaccessible);

            if (offset + length > size_)
                throw Error(ErrorCode::IndexOutOfRange);

            return reinterpret_cast<const Struct*>(data_) + offset;
        }

        Struct* arr(GLuint offset, GLuint length)
        {
            if (!accessible_)
                throw Error(ErrorCode::Inaccessible);

            if (offset + length > size_)
                throw Error(ErrorCode::IndexOutOfRange);
            
            return reinterpret_cast<Struct*>(data_) + offset;
        }

        void grow()
        {
            GLuint new_size = static_cast<GLuint>(Options.grow_multiplier * size_);
            grow(new_size > size_ ? new_size : size_ + 1);
        }

        void grow(GLuint new_size)
        {
            if (!accessible_)
                throw Error(ErrorCode::Inaccessible);
            if (new_size <= size_)
                return;

            GLuint old_size = size_;
            size_ = new_size;

            GLBuffer old_buf;
            std::swap(buf_, old_buf);
            
            glNamedBufferStorage(
                buf_,
                static_cast<GLsizeiptr>(size_ * sizeof(Struct)),
                nullptr,
                GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);
            
            glCopyNamedBufferSubData(
                old_buf,
                buf_,
                0,
                0,
                old_size * sizeof(Struct));
            
            data_ = glMapNamedBufferRange(
                buf_,
                0,
                static_cast<GLsizeiptr>(size_ * sizeof(Struct)),
                GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);

            // no need to unmap old data_, since it uses persistent bit
        }

        void pre_draw(GLuint offset, GLuint length) const
        {
            if (offset + length > size_)
            {
                if (offset >= size_)
                    throw Error(ErrorCode::IndexOutOfRange);
                else
                    length = size_ - 1 - offset;
            }

            glFlushMappedNamedBufferRange(
                buf_,
                static_cast<GLintptr>(offset * sizeof(Struct)),
                static_cast<GLsizeiptr>(length * sizeof(Struct)));
        }

        void pre_draw() const
        {
            glFlushMappedNamedBufferRange(
                buf_,
                static_cast<GLintptr>(0),
                static_cast<GLsizeiptr>(size_ * sizeof(Struct)));
        }

        void post_draw() const
        {
            FenceSync sync;
            if (sync.signaled())
            {
                accessible_ = true;
                return;
            }
            for (GLuint i = 0; i < Options.max_timeout_tries; ++i)
            {
                if (sync.wait(Options.timeout_ns))
                {
                    accessible_ = true;
                    return;
                }

                _OLY_ENGINE_LOG_DEBUG("GRAPHICS") << "Timeout in persistent buffer sync - attempt ("
                    << i << ")" << LOG.nl;
            }

            accessible_ = false;

            _OLY_ENGINE_LOG_ERROR("GRAPHICS") << "Timeout in persistent buffer sync - all attempts failed" << LOG.nl;
            throw Error(ErrorCode::OutOfTime);
        }

        void bind_ssbo_base(GLuint index) const
        {
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, buf_);
        }

        void bind_ubo_base(GLuint index) const
        {
            glBindBufferBase(GL_UNIFORM_BUFFER, index, buf_);
        }
    };

    template<typename... Structs>
    class PersistentGPUBufferBlock
    {
    public:
        static constexpr size_t N = std::tuple_size_v<std::tuple<Structs...>>;

        template<size_t I>
        using StructAlias = std::tuple_element_t<I, std::tuple<Structs...>>;

    private:
        static constexpr internal::PersistentOptions Options = {};
        GLBufferBlock<N> buf_;
        mutable std::array<bool, N> accessible_;
        std::array<GLuint, N> size_;
        std::array<void*, N> data_;

    public:
        PersistentGPUBufferBlock(GLuint size = 0)
        {
            accessible_.fill(true);
            size_.fill(size);
            data_.fill(nullptr);
            init(std::make_index_sequence<N>{});
        }

        PersistentGPUBufferBlock(const std::array<GLuint, N>& sizes)
            : size_(sizes)
        {
            accessible_.fill(true);
            data_.fill(nullptr);
            init(std::make_index_sequence<N>{});
        }

        PersistentGPUBufferBlock(const PersistentGPUBufferBlock&) = delete;
        PersistentGPUBufferBlock(PersistentGPUBufferBlock&&) = delete;

    public:
        template<size_t I>
        GLuint get_size() const
        {
            return size_[I];
        }

        template<size_t I>
        GLuint get_buffer() const
        {
            return buf_[I];
        }

        template<size_t I>
        bool is_accessible() const
        {
            return accessible_[I];
        }

        template<size_t I>
        const StructAlias<I>& at(GLuint i) const
        {
            static_assert(I < N);

            if (!accessible_[I])
                throw Error(ErrorCode::Inaccessible);
            
            if (i >= size_[I])
                throw Error(ErrorCode::IndexOutOfRange);
            
            return reinterpret_cast<const StructAlias<I>*>(data_[I])[i];
        }

        template<size_t I>
        StructAlias<I>& at(GLuint i)
        {
            static_assert(I < N);
            
            if (!accessible_[I])
                throw Error(ErrorCode::Inaccessible);
            
            if (i >= size_[I])
                throw Error(ErrorCode::IndexOutOfRange);
            
            return reinterpret_cast<StructAlias<I>*>(data_[I])[i];
        }

        template<size_t I>
        const StructAlias<I>* arr(GLuint offset, GLuint length) const
        {
            static_assert(I < N);
            
            if (!accessible_[I])
                throw Error(ErrorCode::Inaccessible);
            
            if (offset + length > size_[I])
                throw Error(ErrorCode::IndexOutOfRange);
            
            return reinterpret_cast<const StructAlias<I>*>(data_[I]) + offset;
        }

        template<size_t I>
        StructAlias<I>* arr(GLuint offset, GLuint length)
        {
            static_assert(I < N);
            
            if (!accessible_[I])
                throw Error(ErrorCode::Inaccessible);
            
            if (offset + length > size_[I])
                throw Error(ErrorCode::IndexOutOfRange);
            
            return reinterpret_cast<StructAlias<I>*>(data_[I]) + offset;
        }

        void grow_all()
        {
            grow_impl(std::make_index_sequence<N>{});
        }

        template<size_t I>
        void grow()
        {
            static_assert(I < N);
            GLuint new_size = static_cast<GLuint>(Options.grow_multiplier * size_[I]);
            grow<I>(new_size > size_[I] ? new_size : size_[I] + 1);
        }

        template<size_t I>
        void grow(GLuint new_size)
        {
            static_assert(I < N);
            if (!accessible_[I])
                throw Error(ErrorCode::Inaccessible);
            if (new_size <= size_[I])
                return;

            GLuint old_size = size_[I];
            size_[I] = new_size;

            GLBuffer old_buf;
            buf_.swap<I>(old_buf);

            glNamedBufferStorage(
                buf_[I],
                static_cast<GLsizeiptr>(size_[I] * sizeof(StructAlias<I>)),
                nullptr,
                GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);
            
            glCopyNamedBufferSubData(
                old_buf,
                buf_[I],
                0,
                0,
                old_size * sizeof(StructAlias<I>));
            
            data_[I] = glMapNamedBufferRange(
                buf_[I],
                0,
                static_cast<GLsizeiptr>(size_[I] * sizeof(StructAlias<I>)),
                GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);
            
            // no need to unmap old data_, since it uses persistent bit
        }

        template<size_t I>
        void pre_draw(GLuint offset, GLuint length) const
        {
            static_assert(I < N);

            if (offset + length > size_[I])
            {
                if (offset >= size_[I])
                    throw Error(ErrorCode::IndexOutOfRange);
                else
                    length = size_[I] - 1 - offset;
            }

            glFlushMappedNamedBufferRange(
                buf_[I],
                static_cast<GLintptr>(offset * sizeof(StructAlias<I>)),
                static_cast<GLsizeiptr>(length * sizeof(StructAlias<I>)));
        }

        template<size_t I>
        void pre_draw() const
        {
            static_assert(I < N);

            glFlushMappedNamedBufferRange(
                buf_[I],
                static_cast<GLintptr>(0),
                static_cast<GLsizeiptr>(size_[I] * sizeof(StructAlias<I>)));
        }

        void pre_draw_all() const
        {
            pre_draw_impl(std::make_index_sequence<N>{});
        }

        template<size_t I>
        void post_draw() const
        {
            static_assert(I < N);

            FenceSync sync;
            if (sync.signaled())
            {
                accessible_[I] = true;
                return;
            }
            
            for (GLuint i = 0; i < Options.max_timeout_tries; ++i)
            {
                if (sync.wait(Options.timeout_ns))
                {
                    accessible_[I] = true;
                    return;
                }

                _OLY_ENGINE_LOG_DEBUG("GRAPHICS") << "Timeout in persistent buffer sync - attempt ("
                    << i << ")" << LOG.nl;
            }
            accessible_[I] = false;

            _OLY_ENGINE_LOG_ERROR("GRAPHICS") << "Timeout in persistent buffer sync - all attempts failed" << LOG.nl;
            throw Error(ErrorCode::OutOfTime);
        }

        void post_draw_all() const
        {
            post_draw_impl(std::make_index_sequence<N>{});
        }

        template<size_t I>
        void bind_ssbo_base(GLuint index) const
        {
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, buf_[I]);
        }

        template<size_t I>
        void bind_ubo_base(GLuint index) const
        {
            glBindBufferBase(GL_UNIFORM_BUFFER, index, buf_[I]);
        }

    private:
        template<size_t... Indices>
        void grow_impl(std::index_sequence<Indices...>) const
        {
            (grow<Indices>(), ...);
        }

        template<size_t... Indices>
        void pre_draw_impl(std::index_sequence<Indices...>) const
        {
            (pre_draw<Indices>(), ...);
        }

        template<size_t... Indices>
        void post_draw_impl(std::index_sequence<Indices...>) const
        {
            (post_draw<Indices>(), ...);
        }

        template<size_t... Indices>
        void init(std::index_sequence<Indices...>)
        {
            ((size_[Indices] > 0
                ? (
                    glNamedBufferStorage(
                        buf_[Indices],
                        static_cast<GLsizeiptr>(size_[Indices] * sizeof(StructAlias<Indices>)),
                        nullptr,
                        GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT),

                    data_[Indices] = glMapNamedBufferRange(
                        buf_[Indices],
                        0,
                        static_cast<GLsizeiptr>(size_[Indices] * sizeof(StructAlias<Indices>)),
                        GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT),

                    0)
                : 0
             ), ...);
        }
    };

    template<
        typename Struct,
        internal::PersistentOptions Options = internal::PersistentOptions{}>
    struct LazyPersistentGPUBuffer
    {
        PersistentGPUBuffer<Struct, Options> buf;

    private:
        mutable RangeMerger<GLuint> dirty_;

    public:
        LazyPersistentGPUBuffer(GLuint size_ = 0) : buf(size_) {}

        void flag(GLuint pos) const
        {
            dirty_.insert({ pos, 1 });
        }

        void pre_draw() const
        {
            for (Range<GLuint> range : dirty_)
            {
                try
                {
                    buf.pre_draw(range.initial, range.length);
                }
                catch (const Error& e)
                {
                    if (e.code == ErrorCode::IndexOutOfRange)
                        ; // silently ignore
                    else
                        throw;
                }
            }

            dirty_.clear();
        }

        void post_draw() const
        {
            buf.post_draw();
        }

        void grow()
        {
            buf.grow();
            buf.pre_draw();
            dirty_.clear();
        }

        const Struct& get(GLuint i) const
        {
            return buf[i];
        }

        Struct& set(GLuint i)
        {
            while (i >= buf.get_size())
                grow();

            flag(i);
            return buf[i];
        }

        const Struct* get(GLuint offset, GLuint length) const
        {
            return buf.arr(offset, length);
        }

        Struct* set(GLuint offset, GLuint length)
        {
            while (offset + length > buf.get_size())
                grow();

            for (GLuint i = 0; i < length; ++i)
                flag(offset + i);
            
            return buf.arr(offset, length);
        }
    };

    template<typename... Structs>
    struct LazyPersistentGPUBufferBlock
    {
        static constexpr size_t N = PersistentGPUBufferBlock<Structs...>::N;

        template<size_t I>
        using StructAlias = typename PersistentGPUBufferBlock<Structs...>::template StructAlias<I>;

        PersistentGPUBufferBlock<Structs...> buf;

    private:
        mutable std::array<RangeMerger<GLuint>, N> dirty_;

    public:
        LazyPersistentGPUBufferBlock(GLuint size_ = 0)
            : buf(size_)
        {
        }
        
        LazyPersistentGPUBufferBlock(const std::array<GLuint, N>& sizes)
            : buf(sizes)
        {
        }

    public:
        template<size_t I>
        void flag(GLuint pos) const
        {
            static_assert(I < N);
            dirty_[I].insert({ pos, 1 });
        }

        template<size_t I>
        void pre_draw() const
        {
            static_assert(I < N);

            for (Range<GLuint> range : dirty_[I])
            {
                try
                {
                    buf.pre_draw<I>(range.initial, range.length);
                }
                catch (const Error& e)
                {
                    if (e.code == ErrorCode::IndexOutOfRange)
                        ; // silently ignore
                    else
                        throw;
                }
            }

            dirty_[I].clear();
        }

        void pre_draw_all() const
        {
            pre_draw_impl(std::make_index_sequence<N>{});
        }

        template<size_t I>
        void post_draw() const
        {
            static_assert(I < N); buf.post_draw<I>();
        }

        void post_draw_all() const
        {
            post_draw_impl(std::make_index_sequence<N>{});
        }

        template<size_t I>
        void grow()
        {
            static_assert(I < N);
            buf.grow<I>();
            buf.pre_draw<I>();
            dirty_[I].clear();
        }

        void grow_all() const
        {
            grow_impl(std::make_index_sequence<N>{});
        }

        template<size_t I>
        const StructAlias<I>& get(GLuint i) const
        {
            static_assert(I < N);
            return buf.at<I>(i);
        }

        template<size_t I>
        StructAlias<I>& set(GLuint i, bool* grown = nullptr)
        {
            static_assert(I < N);
            
            while (i >= buf.get_size<I>())
            {
                grow<I>();
                if (grown)
                    *grown = true;
            }

            flag<I>(i);
            return buf.at<I>(i);
        }

        template<size_t I>
        const StructAlias<I>* get(GLuint offset, GLuint length) const
        {
            static_assert(I < N);
            return buf.arr<I>(offset, length);
        }

        template<size_t I>
        StructAlias<I>* set(GLuint offset, GLuint length, bool* grown = nullptr)
        {
            static_assert(I < N);
            while (offset + length > buf.get_size<I>())
            {
                grow<I>();
                if (grown)
                    *grown = true;
            }

            for (GLuint i = 0; i < length; ++i)
                flag<I>(offset + i);
            
            return buf.arr<I>(offset, length);
        }

    private:
        template<size_t... Indices>
        void pre_draw_impl(std::index_sequence<Indices...>) const
        {
            (pre_draw<Indices>(), ...);
        }

        template<size_t... Indices>
        void post_draw_impl(std::index_sequence<Indices...>) const
        {
            (post_draw<Indices>(), ...);
        }

        template<size_t... Indices>
        void grow_impl(std::index_sequence<Indices...>) const
        {
            (grow<Indices>(), ...);
        }
    };
}
