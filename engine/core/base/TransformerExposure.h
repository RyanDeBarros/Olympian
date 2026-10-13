#pragma once

#include "core/base/Transforms.h"

namespace oly
{
    namespace ex
    {
        namespace e
        {
            constexpr bool nonzero(auto param) { return static_cast<bool>(param) != 0; }

            namespace local
            {
                enum Params
                {
                    None = 0,
                    SetLocal = 1,
                    Full = SetLocal
                };

                constexpr Params operator&(Params a, Params b) { return (Params)((int)a & (int)b); }
                constexpr Params operator|(Params a, Params b) { return (Params)((int)a | (int)b); }
            }

            namespace chain
            {
                enum Params
                {
                    None = 0,
                    AttachParent = 1,
                    AttachChild = 2,
                    ClearChildren = 4,
                    AttachOnly = AttachParent | AttachChild,
                    Full = AttachParent | AttachChild | ClearChildren
                };

                constexpr Params operator&(Params a, Params b) { return (Params)((int)a & (int)b); }
                constexpr Params operator|(Params a, Params b) { return (Params)((int)a | (int)b); }
            }

            namespace modifier
            {
                enum Params
                {
                    None = 0,
                    RefModifier = 1,
                    SetModifier = 2,
                    Full = RefModifier | SetModifier
                };

                constexpr Params operator&(Params a, Params b) { return (Params)((int)a & (int)b); }
                constexpr Params operator|(Params a, Params b) { return (Params)((int)a | (int)b); }
            }
        }

        struct ParamsSet
        {
            e::local::Params local = e::local::None;
            e::chain::Params chain = e::chain::None;
            e::modifier::Params modifier = e::modifier::None;

            constexpr bool set_local() const
            {
                return e::nonzero(local & e::local::SetLocal);
            }

            constexpr bool attach_parent() const
            {
                return e::nonzero(chain & e::chain::AttachParent);
            }

            constexpr bool attach_child() const
            {
                return e::nonzero(chain & e::chain::AttachChild);
            }

            constexpr bool clear_children() const
            {
                return e::nonzero(chain & e::chain::ClearChildren);
            }

            constexpr bool set_modifier() const
            {
                return e::nonzero(modifier & e::modifier::SetModifier);
            }

            constexpr bool ref_modifier() const
            {
                return e::nonzero(modifier & e::modifier::RefModifier);
            }
        };

        inline constexpr auto ReadOnly = ParamsSet{
            .local = e::local::None,
            .chain = e::chain::None,
            .modifier = e::modifier::None
        };

        inline constexpr auto Full = ParamsSet{
            .local = e::local::Full,
            .chain = e::chain::Full,
            .modifier = e::modifier::Full
        };
        
        inline constexpr auto AttachOnly = ParamsSet{
            .local = e::local::Full,
            .chain = e::chain::AttachOnly,
            .modifier = e::modifier::Full
        };
    }

	template<ex::ParamsSet Params = ex::ReadOnly>
	struct Transformer2DExposure
	{
	private:
		template<ex::ParamsSet>
		friend struct Transformer2DExposure;

		Transformer2D& transformer;

	public:
		Transformer2DExposure(Transformer2D& transformer) : transformer(transformer) {}

		glm::mat3 global() const
        {
            return transformer.global();
        }

		const Transform2D& get_local() const
        {
            return transformer.get_local();
        }

		UnitVector2D forward() const
        {
            return transformer.forward();
        }

        template<std::derived_from<TransformModifier2D> T>
        const T& get_modifier() const
        {
            return transformer.get_modifier<T>();
        }

		Transform2D& set_local()
            requires (Params.set_local())
		{
			return transformer.set_local();
		}

		void attach_parent(Transformer2D* parent)
			requires (Params.attach_parent())
		{
			transformer.attach_parent(parent);
		}
		
		template<ex::ParamsSet OParams>
		void attach_parent(Transformer2DExposure<OParams>& parent)
			requires (Params.attach_parent() && OParams.attach_child())
		{
			transformer.attach_parent(&parent.transformer);
		}

		void attach_child(Transformer2D& child)
            requires (Params.attach_child())
		{
			transformer.attach_child(child);
		}

		template<ex::ParamsSet OParams>
		void attach_child(Transformer2DExposure<OParams>& child)
			requires (Params.attach_child() && OParams.attach_parent())
		{
			transformer.get_handle().attach_child(&child.transformer);
		}

		void clear_children()
			requires (Params.clear_children())
		{
			transformer.get_handle().clear_children();
		}

		imp::poly<TransformModifier2D>& set_modifier()
			requires (Params.set_modifier())
		{
			return transformer.set_modifier();
		}

		template<std::derived_from<TransformModifier2D> T>
		T& ref_modifier()
			requires (Params.ref_modifier())
		{
			return transformer.ref_modifier<T>();
		}
	};

    template<>
    struct Transformer2DExposure<ex::ReadOnly>
    {
    private:
        template<ex::ParamsSet>
        friend struct Transformer2DExposure;

        const Transformer2D& transformer;

    public:
        Transformer2DExposure(const Transformer2D& transformer) : transformer(transformer) {}

        glm::mat3 global() const
        {
            return transformer.global();
        }

        const Transform2D& get_local() const
        {
            return transformer.get_local();
        }

        UnitVector2D forward() const
        {
            return transformer.forward();
        }

        template<std::derived_from<TransformModifier2D> T>
        const T& get_modifier() const
        {
            return transformer.get_modifier<T>();
        }
    };
}
