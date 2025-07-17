#pragma once

#include "physics/dynamics/materials/Material.h"

#include <stack>
#include <unordered_set>

namespace oly::physics
{
	namespace internal { class MaterialPool; };

	struct MaterialRef
	{
	private:
		friend class internal::MaterialPool;

		bool valid = false;
		size_t pool_idx = size_t(-1);

	public:
		MaterialRef(bool init = false);
		MaterialRef(const MaterialRef&);
		MaterialRef(MaterialRef&&) noexcept;
		MaterialRef& operator=(const MaterialRef&);
		MaterialRef& operator=(MaterialRef&&) noexcept;
		~MaterialRef();

		const Material& operator*() const;
		Material& operator*();
		const Material* operator->() const;
		Material* operator->();

		operator bool() const;
		bool is_valid() const;
		void init();
		void clone();
		MaterialRef get_clone() const;
		void mark_for_deletion() const;
		bool is_marked_for_deletion() const;
		void unmark_for_deletion() const;
		void invalidate();
	};

	// TODO move entirely in MaterialRef.cpp? Then just move MaterialRef into Material.h?
	namespace internal
	{
		// LATER multi-threading and thead safety
		class MaterialPool
		{
			std::vector<Material> materials;
			std::stack<size_t> unoccupied;
			std::unordered_set<size_t> marked_for_deletion;
			std::vector<std::unordered_set<MaterialRef*>> references;

			MaterialPool() = default;
			MaterialPool(const MaterialPool&) = delete;
			MaterialPool(MaterialPool&&) = delete;

		public:
			static MaterialPool& instance()
			{
				static MaterialPool pool;
				return pool;
			}

			// TODO call in frame()
			void clean();

		private:
			friend struct MaterialRef;
			size_t init_slot(const Material& mat = {});
			void mark_for_deletion(size_t idx);
			void unmark_for_deletion(size_t idx);
			bool is_marked_for_deletion(size_t idx) const;
			void increment_references(size_t idx, MaterialRef* mat);
			void decrement_references(size_t idx, MaterialRef* mat);
			void swap_references(size_t idx, MaterialRef* existing, MaterialRef* with);
		};
	}
}
