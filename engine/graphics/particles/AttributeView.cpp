#include "AttributeView.h"

namespace oly::particles
{
	namespace internal
	{
		IAttributeView::IAttributeView(ParticleEmitter& emitter)
			: emitter(emitter)
		{
			AttributeViewManager::instance().views.insert(this);
		}

		IAttributeView::IAttributeView(const IAttributeView& other)
			: emitter(other.emitter)
		{
			AttributeViewManager::instance().views.insert(this);
		}

		IAttributeView::IAttributeView(IAttributeView&& other) noexcept
			: emitter(other.emitter)
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

	void AttributeViewList::unbind_emitter(const ParticleEmitter& emitter)
	{
		for (auto it = views.begin(); it != views.end(); )
		{
			if (&(*it)->emitter == &emitter)
				views.erase(it);
			else
				++it;
		}
	}

	AttributeViewList AttributeViewList::copy(const AttributeViewList& list, const std::vector<ParticleEmitter>& from, std::vector<ParticleEmitter>& to)
	{
		AttributeViewList new_list;
		std::unordered_map<const ParticleEmitter*, size_t> indexes;
		for (const Polymorphic<internal::IAttributeView>& view : list.views)
		{
			const ParticleEmitter* emitter = &view->emitter;

			auto it = indexes.find(emitter);
			if (it == indexes.end())
			{
				for (size_t i = 0; i < from.size(); ++i)
				{
					if (&from[i] == emitter)
					{
						it = indexes.insert({ emitter, i }).first;
						break;
					}
				}

				if (it == indexes.end())
					throw Error(ErrorCode::INCONSISTENT_DATA);
			}

			new_list.views.push_back(view->rebind(to[it->second]));
		}
		return new_list;
	}
}
