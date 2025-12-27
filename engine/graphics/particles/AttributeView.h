#pragma once

#include "graphics/particles/ParticleEmitter.h"

#include <unordered_set>

namespace oly::particles
{
	namespace internal
	{
		// TODO v6 use better name than AttributeView?
		struct IAttributeView
		{
			ParticleEmitter& emitter;

			IAttributeView(ParticleEmitter& emitter);
			IAttributeView(const IAttributeView&);
			IAttributeView(IAttributeView&&) noexcept;
			virtual ~IAttributeView();

			virtual void on_tick() const = 0;
			virtual Polymorphic<IAttributeView> rebind(ParticleEmitter& emitter) const = 0;

			OLY_POLYMORPHIC_CLONE_ABSTACT_DECLARATION(IAttributeView);
		};

		class AttributeViewManager
		{
			friend struct IAttributeView;
			std::unordered_set<IAttributeView*> views;

			AttributeViewManager() = default;
			AttributeViewManager(const AttributeViewManager&) = delete;
			AttributeViewManager(AttributeViewManager&&) = delete;

		public:
			static AttributeViewManager& instance()
			{
				static AttributeViewManager manager;
				return manager;
			}

			void clear()
			{
				views.clear();
			}

			void on_tick();
		};
	}

	template<typename T>
	struct IAttributeOperation
	{
		virtual ~IAttributeOperation() = default;
		virtual void op(const ParticleEmitter& emitter, T& attribute) const {}

		OLY_POLYMORPHIC_CLONE_DEFINITION(IAttributeOperation<T>);
	};

	template<typename T>
	struct AttributeView final : public internal::IAttributeView
	{
		using Attribute = T& (*)(ParticleEmitter&);

		Attribute attribute;
		Polymorphic<IAttributeOperation<T>> op;

		using internal::IAttributeView::IAttributeView;
		AttributeView(ParticleEmitter& emitter, Attribute attribute, Polymorphic<IAttributeOperation<T>>&& op)
			: internal::IAttributeView(emitter), attribute(attribute), op(std::move(op))
		{
		}

		void on_tick() const override { op->op(emitter, attribute(emitter)); }
		
		Polymorphic<IAttributeView> rebind(ParticleEmitter& emitter) const override
		{
			return make_polymorphic<AttributeView<T>>(emitter, attribute, dupl(op));
		}

		OLY_POLYMORPHIC_CLONE_OVERRIDE(AttributeView<T>);
	};

	struct AttributeViewList
	{
		std::vector<Polymorphic<internal::IAttributeView>> views;

		template<typename T>
		void add(AttributeView<T>&& view)
		{
			views.push_back(as_polymorphic<internal::IAttributeView>(std::move(view)));
		}

		void unbind_emitter(const ParticleEmitter& emitter);
		static AttributeViewList copy(const AttributeViewList& list, const std::vector<ParticleEmitter>& from, std::vector<ParticleEmitter>& to);
	};
}
