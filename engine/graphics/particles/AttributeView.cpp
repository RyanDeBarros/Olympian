#include "AttributeView.h"

namespace oly::particles
{
	void internal::AttributeViewManager::on_tick()
	{
		for (IAttributeView* view : views)
			view->on_tick();
	}

	IAttributeView::IAttributeView(ParticleEmitter& emitter, float& attribute)
		: emitter(emitter), attribute(attribute)
	{
		internal::AttributeViewManager::instance().views.insert(this);
	}

	IAttributeView::IAttributeView(const IAttributeView& other)
		: emitter(other.emitter), attribute(other.attribute)
	{
		internal::AttributeViewManager::instance().views.insert(this);
	}

	IAttributeView::IAttributeView(IAttributeView&& other) noexcept
		: emitter(other.emitter), attribute(other.attribute)
	{
		internal::AttributeViewManager::instance().views.insert(this);
	}

	IAttributeView::~IAttributeView()
	{
		internal::AttributeViewManager::instance().views.erase(this);
	}
}
