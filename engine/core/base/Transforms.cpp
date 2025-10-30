#include "Transforms.h"

namespace oly
{
	void internal::Transformer2DRegistry::Handle::init(Transformer2D* transformer)
	{
		internal::Transformer2DRegistry& registry = internal::Transformer2DRegistry::instance();
		id = registry.id_generator.gen();
		if (id == registry.transformers.size())
		{
			registry.transformers.push_back(transformer);
			registry.parent.push_back(NULL_INDEX);
			registry.index_in_parent.push_back(NULL_INDEX);
			registry.children.push_back({});
		}
		else
		{
			registry.transformers[id] = transformer;
			registry.parent[id] = NULL_INDEX;
			registry.index_in_parent[id] = NULL_INDEX;
			registry.children[id].clear();
		}
	}

	void internal::Transformer2DRegistry::Handle::del()
	{
		if (id != NULL_INDEX)
		{
			clear_children();
			unparent();
			internal::Transformer2DRegistry::instance().id_generator.yield(id);
			id = NULL_INDEX;
		}
	}

	internal::Transformer2DRegistry::Handle::Handle(Transformer2D* transformer)
	{
		init(transformer);
	}

	internal::Transformer2DRegistry::Handle::Handle(Transformer2D* transformer, const Handle& other)
	{
		init(transformer);

		if (other.id != NULL_INDEX)
			set_parent(internal::Transformer2DRegistry::instance().parent[other.id]);
	}

	internal::Transformer2DRegistry::Handle::Handle(Transformer2D* transformer, Handle&& other) noexcept
		: id(other.id)
	{
		other.id = NULL_INDEX;

		if (id != NULL_INDEX)
			internal::Transformer2DRegistry::instance().transformers[id] = transformer;
	}

	internal::Transformer2DRegistry::Handle::~Handle()
	{
		del();
	}

	internal::Transformer2DRegistry::Handle& internal::Transformer2DRegistry::Handle::operator=(const Handle& other)
	{
		if (this != &other)
		{
			if (other.id != NULL_INDEX)
			{
				set_parent(internal::Transformer2DRegistry::instance().parent[other.id]);
				clear_children();
			}
			else
				del();
		}
		return *this;
	}

	internal::Transformer2DRegistry::Handle& internal::Transformer2DRegistry::Handle::operator=(Handle&& other) noexcept
	{
		if (this != &other)
		{
			internal::Transformer2DRegistry& registry = internal::Transformer2DRegistry::instance();
			Transformer2D* transformer = id != NULL_INDEX ? registry.transformers[id] : nullptr;

			del();
			id = other.id;
			other.id = NULL_INDEX;

			if (id != NULL_INDEX)
				registry.transformers[id] = transformer;
		}
		return *this;
	}

	void internal::Transformer2DRegistry::Handle::unparent() const
	{
		if (id == NULL_INDEX)
			throw Error(ErrorCode::INVALID_ID);

		internal::Transformer2DRegistry& registry = internal::Transformer2DRegistry::instance();
		const Index parent = registry.parent[id];
		if (parent != NULL_INDEX)
		{
			std::vector<Index>& pc = registry.children[parent];
			const Index index_in_parent = registry.index_in_parent[id];
			pc[index_in_parent] = pc.back();
			pc.pop_back();
			if (index_in_parent < pc.size())
				registry.index_in_parent[pc[index_in_parent]] = index_in_parent;
			registry.transformers[id]->post_set();
		}
	}

	void internal::Transformer2DRegistry::Handle::clear_children() const
	{
		if (id == NULL_INDEX)
			throw Error(ErrorCode::INVALID_ID);

		internal::Transformer2DRegistry& registry = internal::Transformer2DRegistry::instance();
		std::vector<Index>& children = registry.children[id];
		for (Index child : children)
		{
			registry.parent[child] = NULL_INDEX;
			registry.transformers[child]->post_set();
		}
		children.clear();
	}

	Transformer2D* internal::Transformer2DRegistry::Handle::get_parent() const
	{
		if (id == NULL_INDEX)
			throw Error(ErrorCode::INVALID_ID);

		internal::Transformer2DRegistry& registry = internal::Transformer2DRegistry::instance();
		Index parent = registry.parent[id];
		return parent != NULL_INDEX ? registry.transformers[parent] : nullptr;
	}

	void internal::Transformer2DRegistry::Handle::set_parent(Index new_parent) const
	{
		if (id == NULL_INDEX)
			throw Error(ErrorCode::INVALID_ID);

		internal::Transformer2DRegistry& registry = internal::Transformer2DRegistry::instance();
		const Index parent = registry.parent[id];

		if (new_parent == parent)
			return;

		if (parent != NULL_INDEX)
		{
			std::vector<Index>& pc = registry.children[parent];
			const Index index_in_parent = registry.index_in_parent[id];
			pc[index_in_parent] = pc.back();
			pc.pop_back();
			if (index_in_parent < pc.size())
				registry.index_in_parent[pc[index_in_parent]] = index_in_parent;
		}

		if (new_parent != NULL_INDEX)
		{
			registry.parent[id] = new_parent;
			std::vector<Index>& pc = registry.children[new_parent];
			registry.index_in_parent[id] = pc.size();
			pc.push_back(id);
		}

		registry.transformers[id]->post_set();
	}

	void internal::Transformer2DRegistry::Handle::children_post_set_internal() const
	{
		if (id == NULL_INDEX)
			throw Error(ErrorCode::INVALID_ID);

		internal::Transformer2DRegistry& registry = internal::Transformer2DRegistry::instance();
		for (Index child : registry.children[id])
			registry.transformers[child]->post_set_internal();
	}

	void internal::Transformer2DRegistry::Handle::children_post_set_external() const
	{
		if (id == NULL_INDEX)
			throw Error(ErrorCode::INVALID_ID);

		internal::Transformer2DRegistry& registry = internal::Transformer2DRegistry::instance();
		for (Index child : registry.children[id])
			registry.transformers[child]->post_set_external();
	}

	Transformer2D* internal::Transformer2DRegistry::Handle::get_top_level_parent() const
	{
		if (id == NULL_INDEX)
			throw Error(ErrorCode::INVALID_ID);

		Transformer2D* top = get_parent();
		if (!top)
			return internal::Transformer2DRegistry::instance().transformers[id];

		Transformer2D* top_parent = top->get_handle().get_parent();
		while (top_parent)
		{
			top = top_parent;
			top_parent = top ? top->get_handle().get_parent() : nullptr;
		}
		return top;
	}

	void internal::Transformer2DRegistry::Handle::attach_parent(Transformer2D* parent) const
	{
		set_parent(parent ? parent->get_handle().id : NULL_INDEX);
	}

	void internal::Transformer2DRegistry::Handle::attach_child(Transformer2D& child) const
	{
		child.get_handle().set_parent(id);
	}

	internal::Transformer2DRegistry::Index internal::Transformer2DRegistry::Handle::children_count() const
	{
		if (id == NULL_INDEX)
			throw Error(ErrorCode::INVALID_ID);

		return (Index)internal::Transformer2DRegistry::instance().children[id].size();
	}
	
	Transformer2D* internal::Transformer2DRegistry::Handle::get_child(Index index) const
	{
		if (id == NULL_INDEX)
			throw Error(ErrorCode::INVALID_ID);

		internal::Transformer2DRegistry& registry = internal::Transformer2DRegistry::instance();
		return registry.transformers[registry.children[id][index]];
	}

	void internal::Transformer2DRegistry::Handle::break_from_chain() const
	{
		if (id == NULL_INDEX)
			throw Error(ErrorCode::INVALID_ID);

		internal::Transformer2DRegistry& registry = internal::Transformer2DRegistry::instance();
		const Index parent = registry.parent[id];

		if (parent != NULL_INDEX)
		{
			std::vector<Index>& pc = registry.children[parent];
			for (Index child : registry.children[id])
			{
				registry.index_in_parent[child] = pc.size();
				pc.push_back(child);
			}
		}

		for (Index child : registry.children[id])
		{
			registry.parent[child] = parent;
			registry.transformers[child]->post_set();
		}

		registry.children[id].clear();
		unparent();
	}

	Transformer2D::Transformer2D(Transform2D local, Polymorphic<TransformModifier2D>&& modifier)
		: local(local), handle(this), modifier(std::move(modifier))
	{
	}

	Transformer2D::Transformer2D(const Transformer2D& other)
		: local(other.local), handle(this, other.handle), modifier(other.modifier), _global(other._global)
	{
		post_set();
	}

	Transformer2D::Transformer2D(Transformer2D&& other) noexcept
		: local(other.local), handle(this, std::move(other.handle)), modifier(std::move(other.modifier)), _global(other._global),
		_dirty_internal(other._dirty_internal), _dirty_external(other._dirty_external)
	{
	}

	Transformer2D::~Transformer2D()
	{
	}

	Transformer2D& Transformer2D::operator=(const Transformer2D& other)
	{
		if (this != &other)
		{
			handle = other.handle;
			local = other.local;
			modifier = other.modifier;
			_global = other._global;
			_dirty_internal = other._dirty_internal;
			_dirty_external = other._dirty_external;
		}
		return *this;
	}

	Transformer2D& Transformer2D::operator=(Transformer2D&& other) noexcept
	{
		if (this != &other)
		{
			handle = std::move(other.handle);
			local = other.local;
			modifier = std::move(other.modifier);
			_global = other._global;
			_dirty_internal = other._dirty_internal;
			_dirty_external = other._dirty_external;
		}
		return *this;
	}

	void Transformer2D::post_set() const
	{
		post_set_internal();
		post_set_external();
	}

	void Transformer2D::post_set_internal() const
	{
		if (!_dirty_internal)
		{
			_dirty_internal = true;
			handle.children_post_set_internal();
		}
	}

	void Transformer2D::post_set_external() const
	{
		_dirty_external = true;
		handle.children_post_set_external();
	}

	void Transformer2D::pre_get() const
	{
		if (_dirty_internal)
		{
			_dirty_internal = false;
			Transformer2D* parent = handle.get_parent();
			if (parent)
			{
				parent->pre_get();
				_global = local.matrix();
				(*modifier)(_global);
				_global = parent->_global * _global;
			}
			else
			{
				_global = local.matrix();
				(*modifier)(_global);
			}
		}
	}

	bool Transformer2D::flush() const
	{
		bool was_dirty = _dirty_external;
		_dirty_external = false;
		return was_dirty;
	}

	void PivotTransformModifier2D::operator()(glm::mat3& global) const
	{
		global = pivot_matrix(pivot, size) * global;
	}

	void ShearTransformModifier2D::operator()(glm::mat3& global) const
	{
		global = global * shearing_matrix(shearing);
	}

	void OffsetTransformModifier2D::operator()(glm::mat3& global) const
	{
		global = translation_matrix(offset) * global;
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
}
