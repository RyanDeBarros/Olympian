#include "Transforms.h"

namespace oly
{
	Transformer2D::Transformer2D(const Transformer2D& other)
		: local(other.local), modifier(other.modifier->clone()), _global(other._global)
	{
		attach_parent(other.parent);
		post_set();
	}

	Transformer2D::Transformer2D(Transformer2D&& other) noexcept
		: local(other.local), modifier(std::move(other.modifier)), children(std::move(other.children)), _global(other._global), _dirty_internal(other._dirty_internal), _dirty_external(other._dirty_external)
	{
		attach_parent(other.parent);
		other.unparent();
		for (Transformer2D* child : children)
			child->parent = this;
	}

	Transformer2D::~Transformer2D()
	{
		unparent();
		clear_children();
	}

	Transformer2D& Transformer2D::operator=(const Transformer2D& other)
	{
		if (this != &other)
		{
			attach_parent(other.parent);
			local = other.local;
			modifier = other.modifier->clone();
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
			clear_children();
			local = other.local;
			modifier = std::move(other.modifier);
			attach_parent(other.parent);
			other.unparent();
			children = std::move(other.children);
			for (Transformer2D* child : children)
				child->parent = this;
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
			for (Transformer2D* child : children)
				child->post_set_internal();
		}
	}

	void Transformer2D::post_set_external() const
	{
		_dirty_external = true;
		for (Transformer2D* child : children)
			child->post_set_external();
	}

	void Transformer2D::pre_get() const
	{
		if (_dirty_internal)
		{
			_dirty_internal = false;
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

	const Transformer2D* Transformer2D::top_level_parent() const
	{
		const Transformer2D* top = this;
		while (top->parent)
			top = top->parent;
		return top;
	}

	Transformer2D* Transformer2D::top_level_parent()
	{
		Transformer2D* top = this;
		while (top->parent)
			top = top->parent;
		return top;
	}

	void Transformer2D::attach_parent(Transformer2D* new_parent)
	{
		if (new_parent != this)
		{
			if (!new_parent)
				unparent();
			else if (parent != new_parent)
			{
				if (parent)
					parent->children.erase(this);
				parent = new_parent;
				parent->children.insert(this);
				post_set();
			}
		}
	}

	void Transformer2D::insert_chain(Transformer2D* parent_chain)
	{
		if (!parent_chain || parent_chain == this)
			return;
		if (parent)
		{
			parent->children.erase(this);
			Transformer2D* chain_top = parent_chain->top_level_parent();
			parent->children.insert(chain_top);
			chain_top->parent = parent;
		}
		parent = parent_chain;
		parent->children.insert(this);
		post_set();
	}

	void Transformer2D::unparent()
	{
		if (parent)
			parent->children.erase(this);
		parent = nullptr;
		post_set();
	}

	void Transformer2D::clear_children()
	{
		for (Transformer2D* child : children)
		{
			child->parent = nullptr;
			child->post_set();
		}
		children.clear();
	}

	void Transformer2D::pop_from_chain()
	{
		if (parent)
		{
			for (Transformer2D* child : children)
				parent->children.insert(child);
		}
		for (Transformer2D* child : children)
		{
			child->parent = parent;
			child->post_set();
		}
		children.clear();
	}

	Transformer3D::Transformer3D(const Transformer3D& other)
		: local(other.local), modifier(other.modifier->clone()), _global(other._global), _dirty_internal(other._dirty_internal), _dirty_external(other._dirty_external)
	{
		attach_parent(other.parent);
		post_set();
	}

	Transformer3D::Transformer3D(Transformer3D&& other) noexcept
		: local(other.local), modifier(std::move(other.modifier)), children(std::move(other.children)), _global(other._global), _dirty_internal(other._dirty_internal), _dirty_external(other._dirty_external)
	{
		attach_parent(other.parent);
		other.unparent();
		for (Transformer3D* child : children)
			child->parent = this;
	}

	Transformer3D::~Transformer3D()
	{
		unparent();
		clear_children();
	}

	Transformer3D& Transformer3D::operator=(const Transformer3D& other)
	{
		if (this != &other)
		{
			attach_parent(other.parent);
			local = other.local;
			modifier = other.modifier->clone();
			_global = other._global;
			_dirty_internal = other._dirty_internal;
			_dirty_external = other._dirty_external;
		}
		return *this;
	}

	Transformer3D& Transformer3D::operator=(Transformer3D&& other) noexcept
	{
		if (this != &other)
		{
			clear_children();
			local = other.local;
			modifier = std::move(other.modifier);
			attach_parent(other.parent);
			other.unparent();
			children = std::move(other.children);
			for (Transformer3D* child : children)
				child->parent = this;
			_global = other._global;
			_dirty_internal = other._dirty_internal;
			_dirty_external = other._dirty_external;
		}
		return *this;
	}

	void Transformer3D::post_set() const
	{
		post_set_internal();
		post_set_external();
	}

	void Transformer3D::post_set_internal() const
	{
		if (!_dirty_internal)
		{
			_dirty_internal = true;
			for (Transformer3D* child : children)
				child->post_set_internal();
		}
	}

	void Transformer3D::post_set_external() const
	{
		_dirty_external = true;
		for (Transformer3D* child : children)
			child->post_set_external();
	}

	void Transformer3D::pre_get() const
	{
		if (_dirty_internal)
		{
			_dirty_internal = false;
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

	bool Transformer3D::flush() const
	{
		bool was_dirty = _dirty_external;
		_dirty_external = false;
		return was_dirty;
	}

	const Transformer3D* Transformer3D::top_level_parent() const
	{
		const Transformer3D* top = this;
		while (top->parent)
			top = top->parent;
		return top;
	}

	Transformer3D* Transformer3D::top_level_parent()
	{
		Transformer3D* top = this;
		while (top->parent)
			top = top->parent;
		return top;
	}

	void Transformer3D::attach_parent(Transformer3D* new_parent)
	{
		if (new_parent == this)
		{
			if (!new_parent)
				unparent();
			else if (parent != new_parent)
			{
				if (parent)
					parent->children.erase(this);
				parent = new_parent;
				parent->children.insert(this);
				post_set();
			}
		}
	}

	void Transformer3D::insert_chain(Transformer3D* parent_chain)
	{
		if (!parent_chain || parent_chain == this)
			return;
		if (parent)
		{
			parent->children.erase(this);
			Transformer3D* chain_top = parent_chain->top_level_parent();
			parent->children.insert(chain_top);
			chain_top->parent = parent;
		}
		parent = parent_chain;
		parent->children.insert(this);
		post_set();
	}

	void Transformer3D::unparent()
	{
		if (parent)
			parent->children.erase(this);
		parent = nullptr;
		post_set();
	}

	void Transformer3D::clear_children()
	{
		for (Transformer3D* child : children)
		{
			child->parent = nullptr;
			child->post_set();
		}
		children.clear();
	}

	void Transformer3D::pop_from_chain()
	{
		if (parent)
		{
			for (Transformer3D* child : children)
				parent->children.insert(child);
		}
		for (Transformer3D* child : children)
		{
			child->parent = parent;
			child->post_set();
		}
		children.clear();
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
