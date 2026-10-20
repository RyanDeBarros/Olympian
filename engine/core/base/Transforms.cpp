#include "Transforms.h"

#include "core/util/Loader.h"
#include "core/util/Parser.h"
#include "core/util/Polyklass.h"

#include "definitions/Keys.h"

namespace oly
{
    glm::mat2 rotation_matrix_2x2(float rotation)
    {
        float cos = glm::cos(rotation);
        float sin = glm::sin(rotation);
        return { { cos, sin }, { -sin, cos } };
    }

    glm::mat3 rotation_matrix(float rotation)
    {
        float cos = glm::cos(rotation);
        float sin = glm::sin(rotation);
        return { { cos, sin, 0.0f }, { -sin, cos, 0.0f }, vectors::kH3 };
    }

    Transform2D Transform2D::load(TOMLNode node)
    {
        Transform2D transform;
        transform.overload(node);
        return transform;
    }

    void Transform2D::overload(TOMLNode node)
    {
        if (!node)
            return;

        assets::Parser parser(node);

        parser.optional(detail::Key::Position)(position);
        parser.optional(detail::Key::Rotation)(rotation);
        parser.optional(detail::Key::Scale)(scale);
    }

    void TransformModifier2D::overload(imp::poly<TransformModifier2D>& modifier_, TOMLNode node)
    {
        if (auto parser = assets::Parser(node).optional(detail::Key::Modifier).subparser())
        {
            std::string klass = parser->defaulted<std::string>(detail::Key::Klass)();

            _OLY_POLYKLASS_CASES_BEGIN(modifier_)
                _OLY_POLYKLASS_IF_CASE(ShearTransformModifier2D)
                _OLY_POLYKLASS_ELSE_IF_CASE(PivotTransformModifier2D)
                _OLY_POLYKLASS_ELSE_IF_CASE(OffsetTransformModifier2D)
                _OLY_POLYKLASS_CASES_END;

            if (modifier_)
                modifier_->overload(parser->field(assets::kNoKey));
        }
    }

    imp::poly<TransformModifier2D> TransformModifier2D::load(TOMLNode node)
    {
        imp::poly<TransformModifier2D> modifier_ = nullptr;
        overload(modifier_, node);
        return modifier_;
    }

    internal::Transformer2DRegistry::Transformer2DRegistry()
        : id_generator_(0, nmax<Index>() - 1)
    {
    }

    internal::Transformer2DRegistry::Handle::Handle(Transformer2D* transformer)
    {
        init(transformer);
    }

    internal::Transformer2DRegistry::Handle::Handle(Transformer2D* transformer, const Handle& o)
    {
        init(transformer);

        if (o.id_ != kNullIndex)
            set_parent(internal::Transformer2DRegistry::instance().parent_[o.id_]);
    }

    internal::Transformer2DRegistry::Handle::Handle(Transformer2D* transformer, Handle&& o) noexcept
        : id_(o.id_)
    {
        o.id_ = kNullIndex;

        if (id_ != kNullIndex)
            internal::Transformer2DRegistry::instance().transformers_[id_] = transformer;
    }

    internal::Transformer2DRegistry::Handle::~Handle()
    {
        del();
    }

    internal::Transformer2DRegistry::Handle& internal::Transformer2DRegistry::Handle::operator=(const Handle& o)
    {
        if (this != &o)
        {
            if (o.id_ != kNullIndex)
            {
                set_parent(internal::Transformer2DRegistry::instance().parent_[o.id_]);
                clear_children();
            }
            else
                del();
        }
        return *this;
    }

    internal::Transformer2DRegistry::Handle& internal::Transformer2DRegistry::Handle::operator=(Handle&& o) noexcept
    {
        if (this != &o)
        {
            internal::Transformer2DRegistry& registry = internal::Transformer2DRegistry::instance();
            Transformer2D* transformer = id_ != kNullIndex ? registry.transformers_[id_] : nullptr;

            del();
            id_ = o.id_;
            o.id_ = kNullIndex;

            if (id_ != kNullIndex)
                registry.transformers_[id_] = transformer;
        }
        return *this;
    }

    void internal::Transformer2DRegistry::Handle::unparent() const
    {
        if (id_ == kNullIndex)
            throw Error(ErrorCode::InvalidID);

        internal::Transformer2DRegistry& registry = internal::Transformer2DRegistry::instance();
        const Index parent_ = registry.parent_[id_];
        if (parent_ != kNullIndex)
        {
            std::vector<Index>& pc = registry.children_[parent_];
            const Index index_in_parent_ = registry.index_in_parent_[id_];
            pc[index_in_parent_] = pc.back();
            pc.pop_back();
            if (index_in_parent_ < pc.size())
                registry.index_in_parent_[pc[index_in_parent_]] = index_in_parent_;

            registry.transformers_[id_]->post_set();
        }
    }

    void internal::Transformer2DRegistry::Handle::clear_children() const
    {
        if (id_ == kNullIndex)
            throw Error(ErrorCode::InvalidID);

        internal::Transformer2DRegistry& registry = internal::Transformer2DRegistry::instance();
        std::vector<Index>& children_ = registry.children_[id_];
        for (Index child : children_)
        {
            registry.parent_[child] = kNullIndex;
            registry.transformers_[child]->post_set();
        }

        children_.clear();
    }

    Transformer2D* internal::Transformer2DRegistry::Handle::get_parent() const
    {
        if (id_ == kNullIndex)
            throw Error(ErrorCode::InvalidID);

        internal::Transformer2DRegistry& registry = internal::Transformer2DRegistry::instance();
        Index parent_ = registry.parent_[id_];
        return parent_ != kNullIndex ? registry.transformers_[parent_] : nullptr;
    }

    Transformer2D* internal::Transformer2DRegistry::Handle::get_top_level_parent() const
    {
        if (id_ == kNullIndex)
            throw Error(ErrorCode::InvalidID);

        Transformer2D* top = get_parent();
        if (!top)
            return internal::Transformer2DRegistry::instance().transformers_[id_];

        Transformer2D* top_parent = top->get_handle().get_parent();
        while (top_parent)
        {
            top = top_parent;
            top_parent = top ? top->get_handle().get_parent() : nullptr;
        }

        return top;
    }

    void internal::Transformer2DRegistry::Handle::attach_parent(Transformer2D* parent_) const
    {
        set_parent(parent_ ? parent_->get_handle().id_ : kNullIndex);
    }

    void internal::Transformer2DRegistry::Handle::attach_child(Transformer2D& child) const
    {
        child.get_handle().set_parent(id_);
    }

    internal::Transformer2DRegistry::Index internal::Transformer2DRegistry::Handle::children_count() const
    {
        if (id_ == kNullIndex)
            throw Error(ErrorCode::InvalidID);

        return (Index)internal::Transformer2DRegistry::instance().children_[id_].size();
    }

    Transformer2D* internal::Transformer2DRegistry::Handle::get_child(Index index) const
    {
        if (id_ == kNullIndex)
            throw Error(ErrorCode::InvalidID);

        internal::Transformer2DRegistry& registry = internal::Transformer2DRegistry::instance();
        return registry.transformers_[registry.children_[id_][index]];
    }

    void internal::Transformer2DRegistry::Handle::break_from_chain() const
    {
        if (id_ == kNullIndex)
            throw Error(ErrorCode::InvalidID);

        internal::Transformer2DRegistry& registry = internal::Transformer2DRegistry::instance();
        const Index parent_ = registry.parent_[id_];

        if (parent_ != kNullIndex)
        {
            std::vector<Index>& pc = registry.children_[parent_];
            for (Index child : registry.children_[id_])
            {
                registry.index_in_parent_[child] = pc.size();
                pc.push_back(child);
            }
        }

        for (Index child : registry.children_[id_])
        {
            registry.parent_[child] = parent_;
            registry.transformers_[child]->post_set();
        }

        registry.children_[id_].clear();
        unparent();
    }

    void internal::Transformer2DRegistry::Handle::init(Transformer2D* transformer)
    {
        internal::Transformer2DRegistry& registry = internal::Transformer2DRegistry::instance();
        id_ = registry.id_generator_.gen();
        if (id_ == registry.transformers_.size())
        {
            registry.transformers_.push_back(transformer);
            registry.parent_.push_back(kNullIndex);
            registry.index_in_parent_.push_back(kNullIndex);
            registry.children_.push_back({});
        }
        else
        {
            registry.transformers_[id_] = transformer;
            registry.parent_[id_] = kNullIndex;
            registry.index_in_parent_[id_] = kNullIndex;
            registry.children_[id_].clear();
        }
    }

    void internal::Transformer2DRegistry::Handle::del()
    {
        if (id_ != kNullIndex)
        {
            clear_children();
            unparent();
            internal::Transformer2DRegistry::instance().id_generator_.yield(id_);
            id_ = kNullIndex;
        }
    }

    void internal::Transformer2DRegistry::Handle::set_parent(Index new_parent) const
    {
        if (id_ == kNullIndex)
            throw Error(ErrorCode::InvalidID);

        internal::Transformer2DRegistry& registry = internal::Transformer2DRegistry::instance();
        const Index parent_ = registry.parent_[id_];

        if (new_parent == parent_)
            return;

        if (parent_ != kNullIndex)
        {
            std::vector<Index>& pc = registry.children_[parent_];
            const Index index_in_parent_ = registry.index_in_parent_[id_];
            pc[index_in_parent_] = pc.back();
            pc.pop_back();
            if (index_in_parent_ < pc.size())
                registry.index_in_parent_[pc[index_in_parent_]] = index_in_parent_;
        }

        if (new_parent != kNullIndex)
        {
            registry.parent_[id_] = new_parent;
            std::vector<Index>& pc = registry.children_[new_parent];
            registry.index_in_parent_[id_] = pc.size();
            pc.push_back(id_);
        }

        registry.transformers_[id_]->post_set();
    }

    void internal::Transformer2DRegistry::Handle::children_post_set_internal() const
    {
        if (id_ == kNullIndex)
            throw Error(ErrorCode::InvalidID);

        internal::Transformer2DRegistry& registry = internal::Transformer2DRegistry::instance();
        for (Index child : registry.children_[id_])
            registry.transformers_[child]->post_set_internal();
    }

    void internal::Transformer2DRegistry::Handle::children_post_set_external() const
    {
        if (id_ == kNullIndex)
            throw Error(ErrorCode::InvalidID);

        internal::Transformer2DRegistry& registry = internal::Transformer2DRegistry::instance();
        for (Index child : registry.children_[id_])
            registry.transformers_[child]->post_set_external();
    }

    Transformer2D::Transformer2D(Transform2D local_)
        : local_(local_), handle_(this), modifier_(imp::poly_default)
    {
    }

    Transformer2D::Transformer2D(Transform2D local_, imp::poly<TransformModifier2D> modifier_)
        : local_(local_), handle_(this), modifier_(std::move(modifier_))
    {
    }

    Transformer2D::Transformer2D(const Transformer2D& o)
        : local_(o.local_), handle_(this, o.handle_), modifier_(o.modifier_), global_(o.global_)
    {
        post_set();
    }

    Transformer2D::Transformer2D(Transformer2D&& o) noexcept
        : local_(o.local_), handle_(this, std::move(o.handle_)), modifier_(std::move(o.modifier_)), global_(o.global_),
        dirty_internal_(o.dirty_internal_), dirty_external_(o.dirty_external_)
    {
    }

    Transformer2D::~Transformer2D()
    {
    }

    Transformer2D& Transformer2D::operator=(const Transformer2D& o)
    {
        if (this != &o)
        {
            handle_ = o.handle_;
            local_ = o.local_;
            modifier_ = o.modifier_;
            global_ = o.global_;
            dirty_internal_ = o.dirty_internal_;
            dirty_external_ = o.dirty_external_;
        }
        return *this;
    }

    Transformer2D& Transformer2D::operator=(Transformer2D&& o) noexcept
    {
        if (this != &o)
        {
            handle_ = std::move(o.handle_);
            local_ = o.local_;
            modifier_ = std::move(o.modifier_);
            global_ = o.global_;
            dirty_internal_ = o.dirty_internal_;
            dirty_external_ = o.dirty_external_;
        }
        return *this;
    }

    Transformer2D Transformer2D::load(TOMLNode node)
    {
        if (!node)
            return {};

        Transformer2D transformer;
        transformer.set_local() = Transform2D::load(node);
        transformer.set_modifier() = TransformModifier2D::load(node);
        return transformer;
    }

    const internal::Transformer2DRegistry::Handle& Transformer2D::get_handle() const
    {
        return handle_;
    }

    glm::mat3 Transformer2D::global() const
    {
        pre_get();
        return global_;
    }

    void Transformer2D::set_global(const glm::mat3& g)
    {
        global_ = g;
        post_set();
        dirty_internal_ = false;
    }

    bool Transformer2D::flush() const
    {
        bool was_dirty = dirty_external_;
        dirty_external_ = false;
        return was_dirty;
    }

    bool Transformer2D::dirty() const
    {
        return dirty_external_;
    }
    
    const Transform2D& Transformer2D::get_local() const
    {
        return local_;
    }
    
    Transform2D& Transformer2D::set_local()
    {
        post_set();
        return local_;
    }

    UnitVector2D Transformer2D::forward() const
    {
        return UnitVector2D(transform_direction(global(), UnitVector2D::kRight));
    }

    imp::poly<TransformModifier2D>& Transformer2D::set_modifier()
    {
        post_set();
        return modifier_;
    }

    void Transformer2D::attach_parent(Transformer2D* parent) const
    {
        handle_.attach_parent(parent);
    }

    void Transformer2D::attach_child(Transformer2D& child) const
    {
        handle_.attach_child(child);
    }

    void Transformer2D::post_set() const
    {
        post_set_internal();
        post_set_external();
    }

    void Transformer2D::post_set_internal() const
    {
        if (!dirty_internal_)
        {
            dirty_internal_ = true;
            handle_.children_post_set_internal();
        }
    }

    void Transformer2D::post_set_external() const
    {
        dirty_external_ = true;
        handle_.children_post_set_external();
    }

    void Transformer2D::pre_get() const
    {
        if (dirty_internal_)
        {
            dirty_internal_ = false;
            Transformer2D* parent_ = handle_.get_parent();
            if (parent_)
            {
                parent_->pre_get();
                global_ = local_.matrix();
                if (modifier_)
                    (*modifier_)(global_);
                global_ = parent_->global_ * global_;
            }
            else
            {
                global_ = local_.matrix();
                if (modifier_)
                    (*modifier_)(global_);
            }
        }
    }

    PivotTransformModifier2D::PivotTransformModifier2D(glm::vec2 pivot, glm::vec2 size)
        : pivot(pivot)
        , size(size)
    {
    }

    void PivotTransformModifier2D::operator()(glm::mat3& global) const
    {
        global = pivot_matrix(pivot, size) * global;
    }

    void PivotTransformModifier2D::overload(TOMLNode node)
    {
        assets::Parser parser(node);
        parser.optional(detail::Key::Pivot)(pivot);
        parser.optional(detail::Key::Size)(size);
    }

    ShearTransformModifier2D::ShearTransformModifier2D(glm::vec2 shearing)
        : shearing(shearing)
    {
    }

    void ShearTransformModifier2D::operator()(glm::mat3& global) const
    {
        global = global * shearing_matrix(shearing);
    }

    void ShearTransformModifier2D::overload(TOMLNode node)
    {
        assets::Parser(node).optional(detail::Key::Shearing)(shearing);
    }

    OffsetTransformModifier2D::OffsetTransformModifier2D(glm::vec2 offset)
        : offset(offset)
    {
    }

    void OffsetTransformModifier2D::operator()(glm::mat3& global) const
    {
        global = translation_matrix(offset) * global;
    }

    void OffsetTransformModifier2D::overload(TOMLNode node)
    {
        assets::Parser(node).optional(detail::Key::Offset)(offset);
    }

    void CompoundTransformModifier2D<2>::operator()(glm::mat3& global) const
    {
        (*t1)(global);
        (*t2)(global);
    }

    void CompoundTransformModifier2D<3>::operator()(glm::mat3& global) const
    {
        (*t1)(global);
        (*t2)(global);
        (*t3)(global);
    }

    void CompoundTransformModifier2D<4>::operator()(glm::mat3& global) const
    {
        (*t1)(global);
        (*t2)(global);
        (*t3)(global);
        (*t4)(global);
    }

    glm::vec2 transform_point(const glm::mat3& tr, glm::vec2 point)
    {
        return tr * glm::vec3(point, 1.0f);
    }

    glm::vec2 transform_point(const glm::mat3x2& tr, glm::vec2 point)
    {
        return glm::mat2(tr) * point + tr[2];
    }

    glm::vec2 transform_direction(const glm::mat3& tr, glm::vec2 direction)
    {
        return glm::mat2(tr) * direction;
    }

    glm::vec2 transform_direction(const glm::mat3x2& tr, glm::vec2 direction)
    {
        return glm::mat2(tr) * direction;
    }

    glm::vec2 transform_normal(const glm::mat3& tr, glm::vec2 normal)
    {
        return glm::inverse(glm::transpose(glm::mat2(tr))) * normal;
    }

    glm::vec2 transform_normal(const glm::mat3x2& tr, glm::vec2 normal)
    {
        return glm::inverse(glm::transpose(glm::mat2(tr))) * normal;
    }

    glm::mat3 augment(const glm::mat2& l, glm::vec2 t)
    {
        return { glm::vec3(l[0], 0.0f), glm::vec3(l[1], 0.0f), glm::vec3(t, 1.0f) };
    }

    glm::mat3 augment(const glm::mat3x2& tr)
    {
        return { glm::vec3(tr[0], 0.0f), glm::vec3(tr[1], 0.0f), glm::vec3(tr[2], 1.0f) };
    }
}
