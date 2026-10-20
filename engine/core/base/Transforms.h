#pragma once

#include "core/base/Constants.h"
#include "core/base/SmartReference.h"
#include "core/base/UnitVector.h"

#include "external/GLM.h"
#include "external/TOML.h"

#include <imp/dependent_false.hpp>
#include <imp/id_generator.hpp>
#include <imp/polymorphic.hpp>
#include <imp/soft_singleton.hpp>

namespace oly
{
    namespace vectors
    {
        constexpr glm::vec2 kI2 = { 1.0f, 0.0f };
        constexpr glm::vec3 kI3 = { 1.0f, 0.0f, 0.0f };
        constexpr glm::vec4 kI4 = { 1.0f, 0.0f, 0.0f, 0.0f };
        constexpr glm::vec2 kJ2 = { 0.0f, 1.0f };
        constexpr glm::vec3 kJ3 = { 0.0f, 1.0f, 0.0f };
        constexpr glm::vec4 kJ4 = { 0.0f, 1.0f, 0.0f, 0.0f };
        constexpr glm::vec3 kK3 = { 0.0f, 0.0f, 1.0f };
        constexpr glm::vec4 kK4 = { 0.0f, 0.0f, 1.0f, 0.0f };
        constexpr glm::vec3 kH3 = { 0.0f, 0.0f, 1.0f };
        constexpr glm::vec4 kH4 = { 0.0f, 0.0f, 0.0f, 1.0f };
    }

    constexpr glm::mat3 translation_matrix(glm::vec2 position)
    {
        return { vectors::kI3, vectors::kJ3, glm::vec3(position, 1.0f) };
    }

    extern glm::mat2 rotation_matrix_2x2(float rotation);
    extern glm::mat3 rotation_matrix(float rotation);

    constexpr glm::mat3 scale_matrix(glm::vec2 scale)
    {
        return { scale.x * vectors::kI3, scale.y * vectors::kJ3, vectors::kH3 };
    }

    struct Transform2D
    {
        glm::vec2 position = { 0.0f, 0.0f };
        float rotation = 0.0f;
        glm::vec2 scale = { 1.0f, 1.0f };

        static Transform2D load(TOMLNode node);

        constexpr glm::mat3 matrix() const
        {
            return translation_matrix(position) * rotation_matrix(rotation) * scale_matrix(scale);
        }

        void overload(TOMLNode node);
    };

    struct TransformModifier2D
        : public imp::polymorphic
    {
        IMP_POLYMORPHIC_IMPL((TransformModifier2D));

        virtual ~TransformModifier2D() = default;
        virtual void operator()(glm::mat3& global) const {}

        virtual void overload(TOMLNode node) {}

        static void overload(imp::poly<TransformModifier2D>& modifier, TOMLNode node);
        static imp::poly<TransformModifier2D> load(TOMLNode node);
    };

    class Transformer2D;

    namespace internal
    {
        class Transformer2DRegistry final
            : public imp::soft_singleton<Transformer2DRegistry>
        {
            using Index = glm::uint;
            friend class imp::soft_singleton<Transformer2DRegistry>;

        private:
            static const Index kNullIndex = Index(-1);

            imp::soft_id_generator<Index> id_generator_;
            std::vector<Transformer2D*> transformers_;
            std::vector<Index> parent_;
            std::vector<Index> index_in_parent_;
            std::vector<std::vector<Index>> children_;

        private:
            Transformer2DRegistry();

        public:
            class Handle
            {
                friend class Transformer2D;

            private:
                Index id_;

                Handle(Transformer2D* transformer);
                Handle(Transformer2D* transformer, const Handle&);
                Handle(Transformer2D* transformer, Handle&&) noexcept;

            public:
                ~Handle();

            private:
                Handle& operator=(const Handle&);
                Handle& operator=(Handle&&) noexcept;

            public:
                void unparent() const;
                void clear_children() const;
                Transformer2D* get_parent() const;

                Transformer2D* get_top_level_parent() const;

                void attach_parent(Transformer2D* parent) const;
                void attach_child(Transformer2D& child) const;
                Index children_count() const;
                Transformer2D* get_child(Index index) const;

                void break_from_chain() const;

            private:
                void init(Transformer2D* transformer);
                void del();

                void set_parent(Index new_parent) const;
                void children_post_set_internal() const;
                void children_post_set_external() const;
            };
        };
    }

    class Transformer2D
    {
        friend class internal::Transformer2DRegistry::Handle;

    private:
        Transform2D local_;
        internal::Transformer2DRegistry::Handle handle_;
        mutable glm::mat3 global_ = glm::mat3(1.0f);
        mutable bool dirty_internal_ = true;
        mutable bool dirty_external_ = true;
        imp::poly<TransformModifier2D> modifier_;

    public:
        Transformer2D(Transform2D local = {});
        Transformer2D(Transform2D local, imp::poly<TransformModifier2D> modifier);
        Transformer2D(const Transformer2D&);
        Transformer2D(Transformer2D&&) noexcept;
        ~Transformer2D();
        Transformer2D& operator=(const Transformer2D&);
        Transformer2D& operator=(Transformer2D&&) noexcept;

    public:
        static Transformer2D load(TOMLNode node);

        const internal::Transformer2DRegistry::Handle& get_handle() const;

        glm::mat3 global() const;
        void set_global(const glm::mat3& g);

        bool flush() const;
        bool dirty() const;

        const Transform2D& get_local() const;
        Transform2D& set_local();

        UnitVector2D forward() const;

        imp::poly<TransformModifier2D>& set_modifier();

        template<std::derived_from<TransformModifier2D> T>
        const T& get_modifier() const
        {
            return dynamic_cast<const T&>(*modifier_);
        }

        template<std::derived_from<TransformModifier2D> T>
        T& ref_modifier()
        {
            post_set();
            return dynamic_cast<T&>(*modifier_);
        }

        void attach_parent(Transformer2D* parent) const;
        void attach_child(Transformer2D& child) const;

    private:
        void post_set() const;
        void post_set_internal() const;
        void post_set_external() const;
        void pre_get() const;
    };

    using Transformer2DRef = SmartReference<Transformer2D>;

    constexpr glm::mat3 pivot_matrix(glm::vec2 pivot, glm::vec2 size)
    {
        return translation_matrix(size * (glm::vec2(0.5f) - pivot));
    }

    struct PivotTransformModifier2D
        : public TransformModifier2D
    {
        IMP_POLYMORPHIC_IMPL((PivotTransformModifier2D));

        glm::vec2 pivot = { 0.5f, 0.5f };
        glm::vec2 size = { 0.0f, 0.0f };

        PivotTransformModifier2D(glm::vec2 pivot = glm::vec2(0.5f), glm::vec2 size = glm::vec2(0.0f));

        virtual void operator()(glm::mat3& global) const override;
        virtual void overload(TOMLNode node) override;
    };

    constexpr glm::mat3 shearing_matrix(glm::vec2 shearing)
    {
        return { { 1.0f, shearing.y, 0.0f }, { shearing.x, 1.0f, 0.0f }, vectors::kH3 };
    }

    struct ShearTransformModifier2D
        : public TransformModifier2D
    {
        IMP_POLYMORPHIC_IMPL((ShearTransformModifier2D));

        glm::vec2 shearing = { 0.0f, 0.0f };

        ShearTransformModifier2D(glm::vec2 shearing = glm::vec2(0.0f));

        virtual void operator()(glm::mat3& global) const override;
        virtual void overload(TOMLNode node) override;
    };

    struct OffsetTransformModifier2D
        : public TransformModifier2D
    {
        IMP_POLYMORPHIC_IMPL((OffsetTransformModifier2D));

        glm::vec2 offset = { 0.0f, 0.0f };

        OffsetTransformModifier2D(glm::vec2 offset = glm::vec2(0.0f));

        virtual void operator()(glm::mat3& global) const override;
        virtual void overload(TOMLNode node) override;
    };

    template<size_t N>
    struct CompoundTransformModifier2D
    {
        static_assert(imp::dependent_false_v<N>);
    };

    template<>
    struct CompoundTransformModifier2D<2>
        : public TransformModifier2D
    {
        IMP_POLYMORPHIC_IMPL((CompoundTransformModifier2D<2>));

        imp::poly<TransformModifier2D> t1;
        imp::poly<TransformModifier2D> t2;

        virtual void operator()(glm::mat3& global) const override;
    };

    template<>
    struct CompoundTransformModifier2D<3>
        : public TransformModifier2D
    {
        IMP_POLYMORPHIC_IMPL((CompoundTransformModifier2D<3>));

        imp::poly<TransformModifier2D> t1;
        imp::poly<TransformModifier2D> t2;
        imp::poly<TransformModifier2D> t3;

        virtual void operator()(glm::mat3& global) const override;
    };

    template<>
    struct CompoundTransformModifier2D<4>
        : public TransformModifier2D
    {
        IMP_POLYMORPHIC_IMPL((CompoundTransformModifier2D<4>));

        imp::poly<TransformModifier2D> t1;
        imp::poly<TransformModifier2D> t2;
        imp::poly<TransformModifier2D> t3;
        imp::poly<TransformModifier2D> t4;

        virtual void operator()(glm::mat3& global) const override;
    };

    extern glm::vec2 transform_point(const glm::mat3& tr, glm::vec2 point);
    extern glm::vec2 transform_point(const glm::mat3x2& tr, glm::vec2 point);

    extern glm::vec2 transform_direction(const glm::mat3& tr, glm::vec2 direction);
    extern glm::vec2 transform_direction(const glm::mat3x2& tr, glm::vec2 direction);
    
    extern glm::vec2 transform_normal(const glm::mat3& tr, glm::vec2 normal);
    extern glm::vec2 transform_normal(const glm::mat3x2& tr, glm::vec2 normal);

    extern glm::mat3 augment(const glm::mat2& l, glm::vec2 t);
    extern glm::mat3 augment(const glm::mat3x2& tr);
}
