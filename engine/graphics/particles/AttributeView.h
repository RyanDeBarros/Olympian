#pragma once

#include "graphics/particles/ParticleEmitter.h"

#include <unordered_set>

namespace oly::particles
{
	namespace internal
	{
		struct IAttributeView
		{
			IAttributeView();
			IAttributeView(const IAttributeView&);
			IAttributeView(IAttributeView&&) noexcept;
			~IAttributeView();

			virtual void on_tick() const {}

			OLY_POLYMORPHIC_CLONE_DEFINITION(IAttributeView);
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
		virtual void op(const ParticleEmitter& emitter, T& attribute) const {}

		OLY_POLYMORPHIC_CLONE_DEFINITION(IAttributeOperation<T>);
	};

	template<typename T>
	struct AttributeView : public internal::IAttributeView
	{
		const ParticleEmitter& emitter;
		T& attribute;
		Polymorphic<IAttributeOperation<T>> op;

		using internal::IAttributeView::IAttributeView;
		AttributeView(const ParticleEmitter& emitter, T& attribute, Polymorphic<IAttributeOperation<T>>&& op)
			: internal::IAttributeView(), emitter(emitter), attribute(attribute), op(std::move(op))
		{
		}

		void on_tick() const override { op->op(emitter, attribute); }
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
	};
}
