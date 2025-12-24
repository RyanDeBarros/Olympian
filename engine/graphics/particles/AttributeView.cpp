#include "AttributeView.h"

namespace oly::particles
{
	namespace internal
	{
		IAttributeView::IAttributeView()
		{
			AttributeViewManager::instance().views.insert(this);
		}

		IAttributeView::IAttributeView(const IAttributeView& other)
		{
			AttributeViewManager::instance().views.insert(this);
		}

		IAttributeView::IAttributeView(IAttributeView&& other) noexcept
		{
			AttributeViewManager::instance().views.insert(this);
		}

		IAttributeView::~IAttributeView()
		{
			AttributeViewManager::instance().views.erase(this);
		}

		void AttributeViewManager::on_tick()
		{
			for (IAttributeView* view : views)
				view->on_tick();
		}
	}
}
