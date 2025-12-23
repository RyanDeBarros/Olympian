#pragma once

#include "graphics/particles/ParticleEmitter.h"

#include <unordered_set>

namespace oly::particles
{
	struct IAttributeView;

	namespace internal
	{
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

	struct IAttributeView
	{
		ParticleEmitter& emitter;
		float& attribute;

		IAttributeView(ParticleEmitter& emitter, float& attribute);
		IAttributeView(const IAttributeView&);
		IAttributeView(IAttributeView&&) noexcept;
		~IAttributeView();

		virtual void on_tick() = 0;

		OLY_POLYMORPHIC_CLONE_ABSTACT_DECLARATION(IAttributeView);
	};
}
