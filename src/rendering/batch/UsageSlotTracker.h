#pragma once

#include <unordered_map>

#include "core/base/Assert.h"
#include "core/containers/IDGenerator.h"

#include "graphics/backend/specialized/General.h"

namespace oly
{
	namespace rendering
	{
		template<typename StoredObjectType, typename StoredObjectTypeHash = std::hash<StoredObjectType>>
		class UsageSlotTracker
		{
			struct UsageHolder
			{
				StoredObjectType obj;
				GLuint usage = 0;
			};

			std::unordered_map<GLuint, UsageHolder> usages;
			std::unordered_map<StoredObjectType, GLuint, StoredObjectTypeHash> slot_lookup;
			SoftIDGenerator<GLuint> pos_generator;

			void _decrement_usage(GLuint i)
			{
				auto it = usages.find(i);
				--it->second.usage;
				if (it->second.usage == 0)
					erase_slot(it);
			}

			void erase_slot(const typename decltype(usages)::iterator& it)
			{
				pos_generator.yield(it->first); slot_lookup.erase(it->second.obj); usages.erase(it);
			}

		public:
			UsageSlotTracker() { pos_generator.gen(); /* waste 0th slot */ }

			void decrement_usage(GLuint i) { if (i != 0) _decrement_usage(i); }

			bool set_object(LightweightBuffer<Mutability::MUTABLE>& buffer, GLuint& slot, GLuint pos, const StoredObjectType& stored_obj)
			{
				return set_object<StoredObjectType>(buffer, slot, pos, stored_obj, stored_obj);
			}

			template<typename BufferObjectType>
			bool set_object(LightweightBuffer<Mutability::MUTABLE>& buffer, GLuint& slot, GLuint pos, const StoredObjectType& stored_obj, const BufferObjectType& buffer_obj)
			{
				if (stored_obj == StoredObjectType{}) // remove object from sprite
				{
					if (slot != 0)
					{
						_decrement_usage(slot);
						slot = 0;
						return true; // slot has changed
					}
					return false; // slot did not change
				}
				if (slot != 0) // slot references existing object -> decrement its usage
				{
					auto it = usages.find(slot);
					if (stored_obj == it->second.obj) // same object that exists -> do nothing
						return false; // slot did not change
					--it->second.usage;
					if (it->second.usage == 0)
						erase_slot(it);
				}
				auto newit = slot_lookup.find(stored_obj);
				if (newit != slot_lookup.end()) // object already exists -> increment its usage
				{
					++usages.find(newit->second)->second.usage;
					slot = newit->second;
				}
				else // create new object slot
				{
					slot = pos_generator.gen();
					usages[slot] = { stored_obj, 1 };
					slot_lookup[stored_obj] = slot;
					OLY_ASSERT(slot * sizeof(BufferObjectType) <= (GLuint)buffer.get_size());
					if (slot * sizeof(BufferObjectType) == buffer.get_size())
						buffer.grow(buffer.get_size() + sizeof(BufferObjectType));
					buffer.send<BufferObjectType>(slot, buffer_obj);
				}
				return true; // slot has changed
			}

			const StoredObjectType& get_object(GLuint slot) const
			{
				return usages.find(slot)->second.obj;
			}

			StoredObjectType& get_object(GLuint slot)
			{
				return usages.find(slot)->second.obj;
			}

			bool get_slot(const StoredObjectType& obj, GLuint& slot) const
			{
				auto it = slot_lookup.find(obj);
				if (it != slot_lookup.end())
				{
					slot = it->second;
					return true;
				}
				return false;
			}
		};
	}
}
