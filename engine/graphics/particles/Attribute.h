#pragma once

#include "core/util/Parser.h"

#include "external/GLM.h"

#include <imp/polymorphic.hpp>

#include <unordered_set>
#include <functional>
#include <array>

namespace oly::particles
{
    struct ParticleEmitter;

    template<typename T>
    concept ParticleAttribute =
        std::same_as<T, float> ||
        std::same_as<T, glm::vec2> ||
        std::same_as<T, glm::vec3> ||
        std::same_as<T, glm::vec4>;

    enum class SubSelector
    {
        None = 0,
        X = 1,
        R = 1,
        Y = 2,
        G = 2,
        Z = 3,
        B = 3,
        W = 4,
        A = 4,
        XY = 5,
        RG = 5,
        XZ = 6,
        RB = 6,
        XW = 7,
        RA = 7,
        YZ = 8,
        GB = 8,
        YW = 9,
        GA = 9,
        ZW = 10,
        BA = 10,
        XYZ = 11,
        RGB = 11,
        XYW = 12,
        RGA = 12,
        XZW = 13,
        RBA = 13,
        YZW = 14,
        GBA = 14
    };

    template<bool Const>
    class TAttributeSpan
    {
        using PointerType = std::conditional_t<Const, const float*, float*>;

        template<ParticleAttribute T>
        using ArgumentType = std::conditional_t<Const, T, T&>;

        PointerType base_ = nullptr;
        std::array<int8_t, 3> offsets_ = { 0, 0, 0 };
        uint8_t size_ = 0;

    public:
        TAttributeSpan() = default;

        explicit TAttributeSpan(ArgumentType<float> v)
            : base_(&v)
            , offsets_({ 0, 0, 0 })
            , size_(1)
        {
        }

        explicit TAttributeSpan(ArgumentType<glm::vec2> v)
            : base_(glm::value_ptr(v))
            , offsets_({ 1, 0, 0 })
            , size_(2)
        {
        }

        explicit TAttributeSpan(ArgumentType<glm::vec3> v)
            : base_(glm::value_ptr(v))
            , offsets_({ 1, 2, 0 })
            , size_(3)
        {
        }

        explicit TAttributeSpan(ArgumentType<glm::vec4> v)
            : base_(glm::value_ptr(v))
            , offsets_({ 1, 2, 3 })
            , size_(4)
        {
        }

        template<bool OtherConst>
            requires (Const || !OtherConst)
        TAttributeSpan(TAttributeSpan<OtherConst> span)
            : base_(span.base_)
            , offsets_(span.offsets_)
            , size_(span.size_)
        {
        }

        TAttributeSpan& operator=(float v)
            requires (!Const)
        {
            if (size_ == 1)
            {
                *ref(0) = v;
                return *this;
            }
            else
                throw Error(ErrorCode::InvalidSize);
        }

        TAttributeSpan& operator=(glm::vec2 v)
            requires (!Const)
        {
            if (size_ == 2)
            {
                *ref(0) = v.x;
                *ref(1) = v.y;
                return *this;
            }
            else
                throw Error(ErrorCode::InvalidSize);
        }

        TAttributeSpan& operator=(glm::vec3 v)
            requires (!Const)
        {
            if (size_ == 3)
            {
                *ref(0) = v.x;
                *ref(1) = v.y;
                *ref(2) = v.z;
                return *this;
            }
            else
                throw Error(ErrorCode::InvalidSize);
        }

        TAttributeSpan& operator=(glm::vec4 v)
            requires (!Const)
        {
            if (size_ == 4)
            {
                *ref(0) = v.x;
                *ref(1) = v.y;
                *ref(2) = v.z;
                *ref(3) = v.w;
                return *this;
            }
            else
                throw Error(ErrorCode::InvalidSize);
        }

    public:
        template<bool OtherConst = Const>
            requires (!Const || OtherConst)
        TAttributeSpan<OtherConst> select(SubSelector sel)
        {
            TAttributeSpan span;
            switch (sel)
            {
            case SubSelector::None:
                span.base_ = base_;
                span.offsets_ = offsets_;
                span.size_ = size_;
                break;

            case SubSelector::X:
                span.base_ = base_;
                span.size_ = 1;
                break;

            case SubSelector::Y:
                span.base_ = base_ + offsets_[0];
                span.size_ = 1;
                break;

            case SubSelector::Z:
                span.base_ = base_ + offsets_[1];
                span.size_ = 1;
                break;

            case SubSelector::W:
                span.base_ = base_ + offsets_[2];
                span.size_ = 1;
                break;

            case SubSelector::XY:
                span.base_ = base_;
                span.offsets_[0] = offsets_[0];
                span.size_ = 2;
                break;

            case SubSelector::XZ:
                span.base_ = base_;
                span.offsets_[0] = offsets_[1];
                span.size_ = 2;
                break;

            case SubSelector::XW:
                span.base_ = base_;
                span.offsets_[0] = offsets_[2];
                span.size_ = 2;
                break;

            case SubSelector::YZ:
                span.base_ = base_ + offsets_[0];
                span.offsets_[0] = offsets_[1] - offsets_[0];
                span.size_ = 2;
                break;

            case SubSelector::YW:
                span.base_ = base_ + offsets_[0];
                span.offsets_[0] = offsets_[2] - offsets_[0];
                span.size_ = 2;
                break;

            case SubSelector::ZW:
                span.base_ = base_ + offsets_[1];
                span.offsets_[0] = offsets_[2] - offsets_[1];
                span.size_ = 2;
                break;

            case SubSelector::XYZ:
                span.base_ = base_;
                span.offsets_[0] = offsets_[0];
                span.offsets_[1] = offsets_[1];
                span.size_ = 3;
                break;

            case SubSelector::XYW:
                span.base_ = base_;
                span.offsets_[0] = offsets_[0];
                span.offsets_[1] = offsets_[2];
                span.size_ = 3;
                break;

            case SubSelector::XZW:
                span.base_ = base_;
                span.offsets_[0] = offsets_[1];
                span.offsets_[1] = offsets_[2];
                span.size_ = 3;
                break;

            case SubSelector::YZW:
                span.base_ = base_ + offsets_[0];
                span.offsets_[0] = offsets_[1] - offsets_[0];
                span.offsets_[1] = offsets_[2] - offsets_[0];
                span.size_ = 3;
                break;

            default:
                throw Error(ErrorCode::UnsupportedSwitchCase);
            }
            return span;
        }

        float& operator[](size_t i)
            requires (!Const)
        {
            if (i < size_) [[likely]]
                return *ref(i);
            else
                throw Error(ErrorCode::IndexOutOfRange);
        }

        float operator[](size_t i) const
        {
            if (i < size_) [[likely]]
                return *ref(i);
            else
                throw Error(ErrorCode::IndexOutOfRange);
        }

        uint8_t length() const
        {
            return size_;
        }

        float flt() const
        {
            if (size_ == 1)
                return *ref(0);
            else
                throw Error(ErrorCode::InvalidSize);
        }

        glm::vec2 vec2() const
        {
            if (size_ == 2)
                return { *ref(0), *ref(1) };
            else
                throw Error(ErrorCode::InvalidSize);
        }

        glm::vec3 vec3() const
        {
            if (size_ == 3)
                return { *ref(0), *ref(1), *ref(2) };
            else
                throw Error(ErrorCode::InvalidSize);
        }

        glm::vec4 vec4() const
        {
            if (size_ == 4)
                return { *ref(0), *ref(1), *ref(2), *ref(3) };
            else
                throw Error(ErrorCode::InvalidSize);
        }

    private:
        PointerType ref(size_t i) const
        {
            return base_ + (i > 0 ? offsets_[i - 1] : 0);
        }
    };

    using AttributeSpan = TAttributeSpan<false>;
    using ConstAttributeSpan = TAttributeSpan<true>;

    struct IAttributeOperation
        : public imp::polymorphic
    {
        IMP_POLYMORPHIC_IMPL((IAttributeOperation));

    public:
        virtual ~IAttributeOperation() = default;
        virtual void op(const ParticleEmitter& emitter, AttributeSpan attribute) const {}

        static imp::poly<IAttributeOperation> load(TOMLNode node);
    };

    namespace internal
    {
        assets::Parser::Optional<void, void, assets::NullValidator, false> parse_attribute_value(
            const assets::Parser& parser);
    }

    template<ParticleAttribute T>
    struct Attribute
    {
        T value;
        imp::poly<IAttributeOperation> op = nullptr;

        Attribute<T>& operator=(const T& val)
        {
            value = val;
            return *this;
        }

        Attribute<T>& operator=(T&& val)
        {
            value = std::move(val);
            return *this;
        }

        operator T() const
        {
            return value;
        }

        operator T& ()
        {
            return value;
        }
        
        T operator*() const
        {
            return value;
        }
        
        T& operator*()
        {
            return value;
        }
        
        const T* operator->() const
        {
            return &value;
        }
        
        T* operator->()
        {
            return &value;
        }

        auto operator[](size_t i) const
        {
            return value[i];
        }

        auto& operator[](size_t i)
        {
            return value[i];
        }

        void on_tick(const ParticleEmitter& emitter)
        {
            if (op)
                op->op(emitter, AttributeSpan(value));
        }

        void overload(TOMLNode node)
        {
            assets::Parser parser(node);
            internal::parse_attribute_value(parser)(value);
            if (auto operation = IAttributeOperation::load(node))
                op = std::move(operation);
        }

        static Attribute<T> load(TOMLNode node)
        {
            Attribute<T> attribute;
            attribute.overload(node);
            return attribute;
        }
    };

    namespace ops
    {
        template<size_t N>
        struct Sequence
            : public IAttributeOperation
        {
            IMP_POLYMORPHIC_IMPL((Sequence<N>));

        public:
            std::array<imp::poly<IAttributeOperation>, N> ops;

            Sequence() = default;

            template<typename... Ops>
                requires (sizeof...(Ops) == N)
            explicit Sequence(Ops&&... operations)
                : ops{ std::forward<Ops>(operations)... }
            {
            }

            explicit Sequence(std::array<imp::poly<IAttributeOperation>, N>&& operations)
                : ops(std::move(operations))
            {
            }

            void op(const ParticleEmitter& emitter, AttributeSpan attribute) const override
            {
                for (size_t i = 0; i < N; ++i)
                {
                    if (ops[i])
                        ops[i]->op(emitter, attribute);
                }
            }
        };

        template<>
        struct Sequence<0>
            : public IAttributeOperation
        {
            IMP_POLYMORPHIC_IMPL((Sequence<0>));

        public:
            std::vector<imp::poly<IAttributeOperation>> ops;

            Sequence() = default;

            template<typename... Ops>
            explicit Sequence(Ops&&... operations)
                : ops{ std::forward<Ops>(operations)... }
            {
            }

            explicit Sequence(std::vector<imp::poly<IAttributeOperation>>&& operations);

            void op(const ParticleEmitter& emitter, AttributeSpan attribute) const override;

            static imp::poly<Sequence<0>> load(TOMLNode node);
            static imp::poly<IAttributeOperation> load_fixed(TOMLNode node);
        };

        struct Selector
            : public IAttributeOperation
        {
            IMP_POLYMORPHIC_IMPL((Selector));

        public:
            imp::poly<IAttributeOperation> inner_op;

            SubSelector selector = SubSelector::None;

            Selector() = default;
            Selector(const imp::poly<IAttributeOperation>& inner_op, SubSelector selector);
            Selector(imp::poly<IAttributeOperation>&& inner_op, SubSelector selector);

            void op(const ParticleEmitter& emitter, AttributeSpan attribute) const override;

            static imp::poly<Selector> load(TOMLNode node);
        };

        struct GenericFunction
            : public IAttributeOperation
        {
            IMP_POLYMORPHIC_IMPL((GenericFunction));

        public:
            using Function = std::function<void(const ParticleEmitter&, AttributeSpan)>;
            Function fn;

            GenericFunction() = default;
            GenericFunction(Function fn);

            void op(const ParticleEmitter& emitter, AttributeSpan attribute) const override;
        };
    }
}
